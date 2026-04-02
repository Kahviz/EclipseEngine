#include "GLOBALS.h"

#if DIRECTX11 == 1
#include "Dx11Renderer.h"
#include "Releaser.h"
#include <d3dcompiler.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <wrl/client.h>
#include <wincodec.h>
#include <Instances/Instance.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct LightingCB
{
    Vector3 lightpos;     // 12
    float Brightness;      // 4
    Vector3 WorldPos;     // 12
    float lightRange;      // 4
};


void Dx11Renderer::InitDx11Renderer(HWND hWnd)
{
    CreateDeviceAndSwapChain(screen_width, screen_height, hWnd);
    CreateViewport(screen_width, screen_height);
    CreateDepthStencil(screen_width, screen_height);
    CreateRenderTarget();
    pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), pDepthStencilView.Get());

    float AspectX = screen_width;
    float AspectY = screen_height;
    float Aspect = AspectX / AspectY;
    camera.SetProjectionValues(FOV, Aspect, zNear, 1000.0f);

    CreateConstantBuffers();
    CreateShadowResources();

    CompileShaders();

    // Tavallinen sampler
    D3D11_SAMPLER_DESC samp = {};
    samp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samp.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samp.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samp.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samp.MaxLOD = D3D11_FLOAT32_MAX;
    pDevice->CreateSamplerState(&samp, &pSampler);

    // Shadow sampler
    D3D11_SAMPLER_DESC shadowSamp = {};
    shadowSamp.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    shadowSamp.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSamp.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSamp.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    shadowSamp.BorderColor[0] = 1.0f;
    shadowSamp.BorderColor[1] = 1.0f;
    shadowSamp.BorderColor[2] = 1.0f;
    shadowSamp.BorderColor[3] = 1.0f;
    shadowSamp.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    shadowSamp.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT hr = pDevice->CreateSamplerState(&shadowSamp, &pShadowSampler);
    if (FAILED(hr)) throw std::runtime_error("Failed to create shadow sampler");
}

void Dx11Renderer::CreateShadowResources()
{
    D3D11_TEXTURE2D_DESC shadowDesc = {};
    shadowDesc.Width = SHADOW_MAP_SIZE;
    shadowDesc.Height = SHADOW_MAP_SIZE;
    shadowDesc.MipLevels = 1;
    shadowDesc.ArraySize = 1;
    shadowDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // Tärkeä: typeless format
    shadowDesc.SampleDesc.Count = 1;
    shadowDesc.Usage = D3D11_USAGE_DEFAULT;
    shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = pDevice->CreateTexture2D(&shadowDesc, nullptr, &pShadowMap);
    if (FAILED(hr)) throw std::runtime_error("Failed to create shadow map texture");

    // Luo DSV varjokartalle
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = pDevice->CreateDepthStencilView(pShadowMap.Get(), &dsvDesc, &pShadowDSV);
    if (FAILED(hr)) throw std::runtime_error("Failed to create shadow DSV");

    // Luo SRV varjokartalle (shader-resource view)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = pDevice->CreateShaderResourceView(pShadowMap.Get(), &srvDesc, &pShadowSRV);
    if (FAILED(hr)) throw std::runtime_error("Failed to create shadow SRV");

    // Luo rasterizer state varjorenderöintiä varten
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 1000; // Bias estää itsevarjostuksen
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.SlopeScaledDepthBias = 1.0f;
    rasterDesc.DepthClipEnable = TRUE;

    hr = pDevice->CreateRasterizerState(&rasterDesc, &pShadowRasterizer);
    if (FAILED(hr)) throw std::runtime_error("Failed to create shadow rasterizer");

    // Luo constant buffer varjoshaderille
    D3D11_BUFFER_DESC cbd = {};
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.ByteWidth = sizeof(ShadowCB);
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = pDevice->CreateBuffer(&cbd, nullptr, &pShadowCB);
    if (FAILED(hr)) throw std::runtime_error("Failed to create shadow constant buffer");
}

void Dx11Renderer::SetShadowMapToShader()
{
    ID3D11ShaderResourceView* shadowSRV = pShadowSRV.Get();
    pContext->PSSetShaderResources(2, 1, &shadowSRV);

    ID3D11SamplerState* shadowSampler = pShadowSampler.Get();
    pContext->PSSetSamplers(1, 1, &shadowSampler);
}

void Dx11Renderer::RenderShadowMap(std::vector<std::unique_ptr<Instance>>& Drawables)
{
    static float angle = 0.0f;
    float radius = 10.0f;

    angle += 0.01f;
    if (angle >= XM_2PI) angle -= XM_2PI;

    XMFLOAT3 lightPos(radius * cosf(angle), 5.0f, radius * sinf(angle));
    XMVECTOR lightPosition = XMLoadFloat3(&lightPos);
    XMVECTOR lightTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX lightView = XMMatrixLookAtLH(lightPosition, lightTarget, up);
    XMMATRIX lightProj = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1.0f, 1.0f, 50.0f);
    XMMATRIX lightViewProj = lightView * lightProj;

    ShadowCB scb = {};
    scb.lightViewProj = XMMatrixTranspose(lightViewProj);
    XMStoreFloat3(&scb.lightPos, lightPosition);

    D3D11_MAPPED_SUBRESOURCE msr;
    pContext->Map(pShadowCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    memcpy(msr.pData, &scb, sizeof(scb));
    pContext->Unmap(pShadowCB.Get(), 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    pContext->PSSetShaderResources(2, 1, &nullSRV);

    //old
    ID3D11RenderTargetView* oldRTV = nullptr;
    ID3D11DepthStencilView* oldDSV = nullptr;
    pContext->OMGetRenderTargets(1, &oldRTV, &oldDSV);

    ID3D11RasterizerState* oldRasterizer = nullptr;
    pContext->RSGetState(&oldRasterizer);

    ID3D11VertexShader* oldVS = nullptr;
    ID3D11PixelShader* oldPS = nullptr;
    ID3D11ClassInstance* oldVSInstances = nullptr;
    ID3D11ClassInstance* oldPSInstances = nullptr;
    UINT oldVSNumInstances = 0;
    UINT oldPSNumInstances = 0;
    pContext->VSGetShader(&oldVS, &oldVSInstances, &oldVSNumInstances);
    pContext->PSGetShader(&oldPS, &oldPSInstances, &oldPSNumInstances);

    D3D11_VIEWPORT oldViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    UINT numViewports = 1;
    pContext->RSGetViewports(&numViewports, oldViewports);

    // Aseta varjorenderöinnin asetukset
    D3D11_VIEWPORT shadowVP = {};
    shadowVP.Width = static_cast<float>(SHADOW_MAP_SIZE);
    shadowVP.Height = static_cast<float>(SHADOW_MAP_SIZE);
    shadowVP.MinDepth = 0.0f;
    shadowVP.MaxDepth = 1.0f;
    pContext->RSSetViewports(1, &shadowVP);
    pContext->RSSetState(pShadowRasterizer.Get());

    pContext->OMSetRenderTargets(0, nullptr, pShadowDSV.Get());
    pContext->ClearDepthStencilView(pShadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    pContext->VSSetShader(pVS.Get(), nullptr, 0);  // Käytä pVS:ää, ei pShadowVS:ää
    pContext->PSSetShader(nullptr, nullptr, 0);

    pContext->VSSetConstantBuffers(1, 1, pShadowCB.GetAddressOf());

    pContext->IASetInputLayout(pLayout.Get());
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (auto& Instptr : Drawables) {
        Instance& inst = *Instptr.get();
        if (inst.CanDraw()) {
            const Mesh& mesh = inst.OBJmesh;

            XMMATRIX scale = XMMatrixScaling(inst.Size.x(), inst.Size.y(), inst.Size.z());
            XMMATRIX rotation = XMMatrixRotationRollPitchYaw(
                inst.Orientation.x(), inst.Orientation.y(), inst.Orientation.z());
            XMMATRIX translation = XMMatrixTranslation(inst.pos.x(), inst.pos.y(), inst.pos.z());
            XMMATRIX world = scale * rotation * translation;

            XMMATRIX lightViewProj = XMMatrixTranspose(world * lightView * lightProj);

            ConstantBuffer cb = {};
            cb.worldViewProj = lightViewProj;
            cb.world = XMMatrixTranspose(world);
            cb.cubeColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
            cb.padding = 0.0f;

            D3D11_MAPPED_SUBRESOURCE msrVS;
            pContext->Map(pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msrVS);
            memcpy(msrVS.pData, &cb, sizeof(cb));
            pContext->Unmap(pConstantBuffer.Get(), 0);

            pContext->VSSetConstantBuffers(0, 1, pConstantBuffer.GetAddressOf());
            pContext->VSSetConstantBuffers(1, 1, pShadowCB.GetAddressOf());

            mesh.DrawForDX11(pContext.Get());
        }
    }

    pContext->RSSetState(oldRasterizer);
    if (oldRasterizer) oldRasterizer->Release();

    pContext->RSSetViewports(numViewports, oldViewports);

    pContext->VSSetShader(oldVS, &oldVSInstances, oldVSNumInstances);
    pContext->PSGetShader(&oldPS, &oldPSInstances, &oldPSNumInstances);
    if (oldVS) oldVS->Release();
    if (oldPS) oldPS->Release();

    if (oldRTV) {
        pContext->OMSetRenderTargets(1, &oldRTV, oldDSV);
        oldRTV->Release();
    }
    if (oldDSV) oldDSV->Release();
}

void Dx11Renderer::SetRenderTargetToScene() {
    pContext->OMSetRenderTargets(1, pSceneRTV.GetAddressOf(), pDepthStencilView.Get());
}

void Dx11Renderer::SetRenderTargetToBackBuffer() {
    pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), pDepthStencilView.Get());
}

void Dx11Renderer::CreateDeviceAndSwapChain(int width, int height, HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.SampleDesc.Count = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = 1;
    scd.OutputWindow = hWnd;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scd.Flags = 0;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    D3D_FEATURE_LEVEL selectedFeatureLevel;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &scd,
        &pSwap,
        &pDevice,
        &selectedFeatureLevel,
        &pContext
    );

    if (FAILED(hr)) {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &scd,
            &pSwap,
            &pDevice,
            &selectedFeatureLevel,
            &pContext
        );

        if (FAILED(hr))
            throw std::runtime_error("Failed to create D3D11 device and context");
    }
}

void Dx11Renderer::CreateViewport(int width, int height)
{
    D3D11_VIEWPORT vp = {};

    vp.Width = static_cast<float>(width);
    vp.Height = static_cast<float>(height);

    vp.TopLeftX = (static_cast<float>(width) - vp.Width) / 2.0f;
    vp.TopLeftY = (static_cast<float>(height) - vp.Height) / 2.0f;

    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    pContext->RSSetViewports(1, &vp);
}

void Dx11Renderer::CreateDepthStencil(int width, int height)
{
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ComPtr<ID3D11Texture2D> pDepthStencil;
    HRESULT hr = pDevice->CreateTexture2D(&depthDesc, nullptr, &pDepthStencil);
    if (FAILED(hr)) throw std::runtime_error("Failed to create depth stencil texture");

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = pDevice->CreateDepthStencilView(pDepthStencil.Get(), &dsvDesc, &pDepthStencilView);
    if (FAILED(hr)) throw std::runtime_error("Failed to create depth stencil view");

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

    hr = pDevice->CreateDepthStencilState(&dsDesc, &pDepthStencilState);
    if (FAILED(hr)) throw std::runtime_error("Failed to create depth stencil state");

    pContext->OMSetDepthStencilState(pDepthStencilState.Get(), 1);
}

void Dx11Renderer::CreateRenderTarget()
{
    ComPtr<ID3D11Texture2D> pBackBuffer;
    HRESULT hr = pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), &pBackBuffer);
    if (FAILED(hr)) throw std::runtime_error("Failed to get back buffer");

    hr = pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget);
    if (FAILED(hr)) throw std::runtime_error("Failed to create render target view");
}

void Dx11Renderer::CreateConstantBuffers()
{
    // VS constant buffer
    D3D11_BUFFER_DESC cbd = {};
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0;
    cbd.StructureByteStride = 0;

    HRESULT hr = pDevice->CreateBuffer(&cbd, nullptr, &pConstantBuffer);
    if (FAILED(hr)) throw std::runtime_error("Failed to create constant buffer");

    // PS Color buffer (b0)
    D3D11_BUFFER_DESC pcbd = {};
    pcbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    pcbd.ByteWidth = sizeof(PixelConstantBuffer);
    pcbd.Usage = D3D11_USAGE_DYNAMIC;
    pcbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    pcbd.MiscFlags = 0;
    pcbd.StructureByteStride = 0;

    hr = pDevice->CreateBuffer(&pcbd, nullptr, &pColorBuffer);
    if (FAILED(hr)) throw std::runtime_error("Failed to create pixel constant buffer");

    D3D11_BUFFER_DESC lcbd = {};
    lcbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lcbd.ByteWidth = sizeof(LightingCB);
    lcbd.Usage = D3D11_USAGE_DYNAMIC;
    lcbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    lcbd.MiscFlags = 0;
    lcbd.StructureByteStride = 0;

    hr = pDevice->CreateBuffer(&lcbd, nullptr, &pLightingBuffer);
    if (FAILED(hr)) throw std::runtime_error("Failed to create lighting constant buffer");
}

void Dx11Renderer::CompileShaders()
{
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;
    ComPtr<ID3DBlob> psTextureBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompileFromFile(
        L"VertexShader.hlsl",
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &vsBlob,
        nullptr
    );

    if (FAILED(hr)) {
        hr = D3DCompileFromFile(
            L"VertexShader.hlsl",
            nullptr,
            nullptr,
            "main",
            "vs_5_0",
            0,
            0,
            &vsBlob,
            nullptr
        );
    }
    if (FAILED(hr)) throw std::runtime_error("Failed to compile Vertex shader");

    hr = pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &pVS);
    if (FAILED(hr)) throw std::runtime_error("Failed to create Vertex shader");

    hr = D3DCompileFromFile(
        L"PixelShaderTexture.hlsl",
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &psTextureBlob,
        &errorBlob
    );

    bool textureShaderExists = SUCCEEDED(hr);

    if (textureShaderExists) {
        hr = pDevice->CreatePixelShader(psTextureBlob->GetBufferPointer(), psTextureBlob->GetBufferSize(), nullptr, &pPSTexture);
        if (FAILED(hr)) textureShaderExists = false;
    }

    hr = D3DCompileFromFile(
        L"PixelShader.hlsl",
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &psBlob,
        &errorBlob
    );

    if (FAILED(hr)) {
        hr = D3DCompileFromFile(
            L"PixelShader.hlsl",
            nullptr,
            nullptr,
            "main",
            "ps_5_0",
            0,
            0,
            &psBlob,
            nullptr
        );
    }

    if (FAILED(hr)) throw std::runtime_error("Failed to compile pixel shader");

    hr = pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pPSNoTexture);
    if (FAILED(hr)) throw std::runtime_error("Failed to create pixel shader");

    pPS = pPSNoTexture;

    if (textureShaderExists) {

    }

    D3D11_INPUT_ELEMENT_DESC ied[] = {
        {"BRIGHTNESS", 0, DXGI_FORMAT_R32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    hr = pDevice->CreateInputLayout(
        ied,
        ARRAYSIZE(ied),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &pLayout
    );

    if (FAILED(hr)) throw std::runtime_error("Failed to create input layout");
}

void Dx11Renderer::ReSizeWindow(int width, int height, HWND hWnd)
{
    if (!pSwap) return;

    pTarget.Reset();
    pDepthStencilView.Reset();
    pContext->ClearState();
    pContext->Flush();

    HRESULT hr = pSwap->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        CreateDeviceAndSwapChain(width, height, hWnd);
        CreateRenderTarget();
    }
    else {
        CreateRenderTarget();
    }

    CreateViewport(width, height);
    CreateDepthStencil(width, height);
    pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), pDepthStencilView.Get());

    float AspectX = width;
    float AspectY = height;

#if INEDITOR == 1
    AspectX = viewport_width;
    AspectY = viewport_height;
#endif
    float Aspect = AspectX / AspectY;

    viewport_height = AspectY;
    viewport_width = AspectX;

    CreateSceneResources(width, height);
    camera.SetProjectionValues(FOV, Aspect, zNear, 1000.0f);
}


void Dx11Renderer::CreateSceneResources(int width, int height) {
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    pDevice->CreateTexture2D(&td, nullptr, &pSceneTexture);

    pDevice->CreateRenderTargetView(pSceneTexture.Get(), nullptr, &pSceneRTV);

    pDevice->CreateShaderResourceView(pSceneTexture.Get(), nullptr, &pSceneSRV);
}

ID3D11ShaderResourceView* Dx11Renderer::GetSceneSRV() {
    return pSceneSRV.Get();
}

ID3D11RenderTargetView* Dx11Renderer::GetBackBufferRTV()
{
    return pSceneRTV.Get();
}

Dx11Renderer::~Dx11Renderer()
{
}

Camera& Dx11Renderer::GetCamera()
{
    return camera;
}

ID3D11Device* Dx11Renderer::GetDevice() noexcept
{
    return pDevice.Get();
}

ID3D11DeviceContext* Dx11Renderer::GetpContext() noexcept
{
    return pContext.Get();
}

void Dx11Renderer::EndFrame()
{
    if (!pSwap) return;

    HRESULT hr = pSwap->Present(vSync, 0);

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        throw std::runtime_error("DirectX device lost");
    }
}

void Dx11Renderer::DrawAFrame(float deltatime, std::vector<std::unique_ptr<Instance>>& Drawables)
{
    RenderShadowMap(Drawables);
    SetShadowMapToShader();

    for (auto& Instptr : Drawables) {
        Instance& inst = *Instptr.get();

        ID3D11PixelShader* selectedPS = pPSNoTexture.Get();

        Texture* tex = inst.GetTexture();
        bool hasTexture = tex->IsLoaded();

        if (hasTexture && tex->GetTextureComPtr() != nullptr) {
            selectedPS = pPSTexture.Get();
            ID3D11ShaderResourceView* textureSRV = tex->GetTextureComPtr().Get();
            pContext->PSSetShaderResources(0, 1, &textureSRV);
        }
        else {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            pContext->PSSetShaderResources(1, 1, &nullSRV);
        }

        pContext->VSSetShader(pVS.Get(), nullptr, 0);
        pContext->PSSetShader(selectedPS, nullptr, 0);
        pContext->PSSetSamplers(0, 1, pSampler.GetAddressOf());

        if (inst.CanDraw()) {
            const Mesh& mesh = inst.OBJmesh;
            Vector3 Orientation = inst.Orientation;
            Vector3& pos = inst.pos;
            Vector3& size = inst.Size;
            Int3 color = inst.color;
            float Brightness = 1.0f;

            // Matriisit
            XMMATRIX scale = XMMatrixScaling(size.x(), size.y(), size.z());
            XMMATRIX rotation = XMMatrixRotationRollPitchYaw(Orientation.x(), Orientation.y(), Orientation.z());
            XMMATRIX translation = XMMatrixTranslation(pos.x(), pos.y(), pos.z());
            XMMATRIX world = scale * rotation * translation;  // world-matriisi

            // Kamera matriisit
            Matrix4x4 mat = camera.GetViewMatrix();
            Matrix4x4 projMat = camera.GetProjectionMatrix();

            XMMATRIX proj = XMMATRIX(
                XMVectorSet(projMat(0, 0), projMat(0, 1), projMat(0, 2), projMat(0, 3)),
                XMVectorSet(projMat(1, 0), projMat(1, 1), projMat(1, 2), projMat(1, 3)),
                XMVectorSet(projMat(2, 0), projMat(2, 1), projMat(2, 2), projMat(2, 3)),
                XMVectorSet(projMat(3, 0), projMat(3, 1), projMat(3, 2), projMat(3, 3))
            );

            XMMATRIX view = XMMATRIX(
                XMVectorSet(mat(0, 0), mat(1, 0), mat(2, 0), mat(3, 0)),  // Sarake 0
                XMVectorSet(mat(0, 1), mat(1, 1), mat(2, 1), mat(3, 1)),  // Sarake 1
                XMVectorSet(mat(0, 2), mat(1, 2), mat(2, 2), mat(3, 2)),  // Sarake 2
                XMVectorSet(mat(0, 3), mat(1, 3), mat(2, 3), mat(3, 3))   // Sarake 3
            );
            XMMATRIX worldViewProj = world * view * proj;

            // VS constant buffer
            ConstantBuffer cb = {};
            cb.worldViewProj = XMMatrixTranspose(worldViewProj);
            cb.world = XMMatrixTranspose(world);  // Lähetä world-matriisi
            cb.cubeColor = XMFLOAT3(
                color.x() / 255.0f,
                color.y() / 255.0f,
                color.z() / 255.0f
            );
            cb.padding = 0.0f;

            D3D11_MAPPED_SUBRESOURCE msrVS;
            HRESULT hr = pContext->Map(pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msrVS);
            if (FAILED(hr)) throw std::runtime_error("Failed to map VS constant buffer");
            memcpy(msrVS.pData, &cb, sizeof(cb));
            pContext->Unmap(pConstantBuffer.Get(), 0);

            // PS Color buffer
            PixelConstantBuffer pcb = {};
            pcb.color = XMFLOAT4(color.x() / 255.f, color.y() / 255.f, color.z() / 255.f, 1.0f);

            D3D11_MAPPED_SUBRESOURCE msrColor;
            hr = pContext->Map(pColorBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msrColor);
            if (FAILED(hr)) throw std::runtime_error("Failed to map color buffer");
            memcpy(msrColor.pData, &pcb, sizeof(pcb));
            pContext->Unmap(pColorBuffer.Get(), 0);

            // Lighting (käytä samaa valoa kuin shadow mapissa)
            static float angle = 0.0f;
            const float radius = 10.0f;
            const float speed = 0.01f;

            angle += speed;
            if (angle >= XM_2PI) angle -= XM_2PI;

            XMFLOAT3 lightpos;
            lightpos.x = radius * cosf(angle);
            lightpos.z = radius * sinf(angle);
            lightpos.y = 5.0f;

            LightingCB lcb = {};
            lcb.lightpos = Vector3(lightpos.x, lightpos.y, lightpos.z);
            lcb.Brightness = Brightness;
            lcb.WorldPos = Vector3(pos.x(), pos.y(), pos.z());
            lcb.lightRange = 20.0f;

            D3D11_MAPPED_SUBRESOURCE msrLighting;
            hr = pContext->Map(pLightingBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msrLighting);
            if (FAILED(hr)) throw std::runtime_error("Failed to map lighting buffer");
            memcpy(msrLighting.pData, &lcb, sizeof(lcb));
            pContext->Unmap(pLightingBuffer.Get(), 0);

            // Aseta constant bufferit
            pContext->VSSetConstantBuffers(0, 1, pConstantBuffer.GetAddressOf());
            pContext->PSSetConstantBuffers(0, 1, pColorBuffer.GetAddressOf());
            pContext->PSSetConstantBuffers(1, 1, pLightingBuffer.GetAddressOf());

            pContext->IASetInputLayout(pLayout.Get());
            pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            mesh.DrawForDX11(pContext.Get());
        }
    }
}

void Dx11Renderer::ClearSceneBuffer(float r, float g, float b)
{
    if (!pContext || !pSceneRTV || !pDepthStencilView)
        return;

    const float color[] = { r, g, b, 1.0f };

    pContext->ClearRenderTargetView(pSceneRTV.Get(), color);

    pContext->ClearDepthStencilView(pDepthStencilView.Get(),
        D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Dx11Renderer::ClearBuffer(float r, float g, float b)
{
    if (!pContext || !pTarget || !pDepthStencilView)
        return;

    const float color[] = { r, g, b, 1.0f };

    pContext->ClearRenderTargetView(pTarget.Get(), color);
    pContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}
#endif