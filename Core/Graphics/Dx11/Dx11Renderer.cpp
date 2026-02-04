#include "Dx11Renderer.h"
#include "Releaser.h"
#include <d3dcompiler.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include "GLOBALS.h"
#include <wrl/client.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct LightingCB
{
    FLOAT3 lightpos;     // 12
    float Brightness;      // 4
    FLOAT3 WorldPos;     // 12
    float lightRange;      // 4
};


void Dx11Renderer::InitDx11Renderer(HWND hWnd)
{
    CreateDeviceAndSwapChain(screen_width, screen_height, hWnd);

    CreateViewport(screen_width, screen_height);

    CreateDepthStencil(screen_width, screen_height);

    CreateRenderTarget();

    pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), pDepthStencilView.Get());

    camera.SetPosition(0.0f, 0.0f, -5.0f);
    float AspectX = screen_width;
    float AspectY = screen_height;
    float Aspect = AspectX / AspectY;

    camera.SetProjectionValues(FOV, Aspect, 0.5f, 1000.0f);

    CreateConstantBuffers();

    CompileShaders();
}

void Dx11Renderer::SetRenderTargetToScene() {
    pContext->OMSetRenderTargets(1, pSceneRTV.GetAddressOf(), pDepthStencilView.Get());
}

void Dx11Renderer::SetRenderTargetToBackBuffer() {
    pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), nullptr);
}

void Dx11Renderer::CreateDeviceAndSwapChain(int width, int height, HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
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
        // Yritä ilman debug flagia
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

    // Lighting buffer (b1)
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

    // Vertex shader
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
        // Yritä ilman debug flagia
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

    // Pixel shader
    hr = D3DCompileFromFile(
        L"PixelShader.hlsl",
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &psBlob,
        nullptr
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

    hr = pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &pVS);
    if (FAILED(hr)) throw std::runtime_error("Failed to create Vertex shader");

    hr = pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pPS);
    if (FAILED(hr)) throw std::runtime_error("Failed to create pixel shader");

    D3D11_INPUT_ELEMENT_DESC ied[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
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
    camera.SetProjectionValues(FOV, Aspect, 0.5f, 1000.0f);
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

    HRESULT hr = pSwap->Present(vSync ? 1 : 0, 0);

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        throw std::runtime_error("DirectX device lost");
    }
}

void Dx11Renderer::DrawAFrame(float deltatime, std::vector<std::unique_ptr<Instance>>& Drawables)
{
    for (auto& Instptr : Drawables) {
        Instance& inst = *Instptr.get();

        if (inst.CanDraw()) {
            const Mesh& mesh = inst.OBJmesh;
            FLOAT3 Orientation = inst.Orientation;
            FLOAT3& pos = inst.pos;
            FLOAT3& size = inst.Size;
            INT3 color = inst.color;
            FLOAT3& Velocity = inst.Velocity;
            bool Anchored = inst.Anchored;
            float Roughness = 1.0f;
            float Brightness = 1.0f;

            if (!pContext || !pConstantBuffer || !pColorBuffer || !pLightingBuffer)
                throw std::runtime_error("Dx11Renderer not properly initialized");

            // Physics
            float restitution = std::clamp(
                (size.x + size.y + size.z) / 6.0f,
                0.1f,
                0.9f
            );

            if (Running && !Anchored)
            {
                // Gravity
                Velocity.y -= Gravity * deltatime;

                // Integrate
                pos.x += Velocity.x * deltatime;
                pos.y += Velocity.y * deltatime;
                pos.z += Velocity.z * deltatime;

                // Ground collision
                if (pos.y < -10.0f)
                {
                    pos.y = -10.0f;
                    Velocity.y = -Velocity.y * restitution;

                    float friction = 0.9f;
                    Velocity.x *= friction;
                    Velocity.z *= friction;

                    if (std::abs(Velocity.y) < 0.05f)
                        Velocity.y = 0.0f;
                }
            }

            // Matrices
            XMMATRIX scale = XMMatrixScaling(size.x, size.y, size.z);
            XMMATRIX world = scale *
                XMMatrixRotationRollPitchYaw(Orientation.x, Orientation.y, Orientation.z) *
                XMMatrixTranslation(pos.x, pos.y, pos.z);

            Matrix4x4 mat = camera.GetViewMatrix();
            XMMATRIX view = XMMATRIX(
                XMVectorSet(mat.x.x, mat.x.y, mat.x.z, mat.x.w),
                XMVectorSet(mat.y.x, mat.y.y, mat.y.z, mat.y.w),
                XMVectorSet(mat.z.x, mat.z.y, mat.z.z, mat.z.w),
                XMVectorSet(mat.w.x, mat.w.y, mat.w.z, mat.w.w)
            );

            Matrix4x4 projMat = camera.GetProjectionMatrix();
            XMMATRIX proj = XMMATRIX(
                XMVectorSet(projMat.x.x, projMat.x.y, projMat.x.z, projMat.x.w),
                XMVectorSet(projMat.y.x, projMat.y.y, projMat.y.z, projMat.y.w),
                XMVectorSet(projMat.z.x, projMat.z.y, projMat.z.z, projMat.z.w),
                XMVectorSet(projMat.w.x, projMat.w.y, projMat.w.z, projMat.w.w)
            );

            XMMATRIX transform = XMMatrixTranspose(world * view * proj);

            // VS constant buffer
            ConstantBuffer cb = {};
            cb.transfrom = transform;
            cb.cubeColor = XMFLOAT3(
                color.x / 255.0f,
                color.y / 255.0f,
                color.z / 255.0f
            );

            D3D11_MAPPED_SUBRESOURCE msrVS;
            HRESULT hr = pContext->Map(pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msrVS);
            if (FAILED(hr)) throw std::runtime_error("Failed to map VS constant buffer");
            memcpy(msrVS.pData, &cb, sizeof(cb));
            pContext->Unmap(pConstantBuffer.Get(), 0);

            // PS Color buffer (b0)
            PixelConstantBuffer pcb = {};
            pcb.color = XMFLOAT4(color.x / 255.f, color.y / 255.f, color.z / 255.f, 1.0f);

            D3D11_MAPPED_SUBRESOURCE msrColor;
            hr = pContext->Map(pColorBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msrColor);
            if (FAILED(hr)) throw std::runtime_error("Failed to map color buffer");
            memcpy(msrColor.pData, &pcb, sizeof(pcb));
            pContext->Unmap(pColorBuffer.Get(), 0);

            // Lighting
            static float angle = 0.0f;
            const float radius = 5.0f;
            const float speed = 0.0001f;

            angle += speed;
            if (angle >= XM_2PI)
                angle -= XM_2PI;

            XMFLOAT3 lightpos;
            lightpos.x = radius * cosf(angle);
            lightpos.z = radius * sinf(angle);
            lightpos.y = 2.0f;

            LightingCB lcb = {};
            lcb.lightpos = FLOAT3(lightpos.x, lightpos.y, lightpos.z);
            lcb.Brightness = Brightness;
            lcb.WorldPos = FLOAT3(pos.x, pos.y, pos.z);
            lcb.lightRange = 10.0f;

            D3D11_MAPPED_SUBRESOURCE msrLighting;
            hr = pContext->Map(pLightingBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msrLighting);
            if (FAILED(hr)) throw std::runtime_error("Failed to map lighting buffer");
            memcpy(msrLighting.pData, &lcb, sizeof(lcb));
            pContext->Unmap(pLightingBuffer.Get(), 0);

            // Pipeline
            pContext->IASetInputLayout(pLayout.Get());
            pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            pContext->VSSetShader(pVS.Get(), nullptr, 0);
            pContext->PSSetShader(pPS.Get(), nullptr, 0);

            pContext->VSSetConstantBuffers(0, 1, pConstantBuffer.GetAddressOf());
            pContext->PSSetConstantBuffers(0, 1, pColorBuffer.GetAddressOf());
            pContext->PSSetConstantBuffers(1, 1, pLightingBuffer.GetAddressOf());

            mesh.DrawForDX11(pContext.Get());
        }
    }
}

void Dx11Renderer::DrawMesh(
    float deltaTime,
    Mesh& mesh,
    FLOAT3 Orientation,
    FLOAT3& pos,
    FLOAT3& size,
    INT3 color,
    FLOAT3& Velocity,
    bool Anchored,
    float Roughness,
    float Brightness
)
{
    
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