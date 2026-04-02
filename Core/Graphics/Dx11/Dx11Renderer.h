#include "GLOBALS.h"

#if DIRECTX11 == 1
#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include "Camera/Camera.h"
#include <wrl/client.h>
#include <Instances/Instance.h>
#include "GLOBALS.h"
#include "Graphics/Texture/Texture.h"
#include "UGE_ASSERTS.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct ConstantBuffer
{
    XMMATRIX worldViewProj;  // world * view * proj
    XMMATRIX world;          // Pelkkä world-matriisi
    XMFLOAT3 cubeColor;
    float padding;
};
struct PixelConstantBuffer
{
    XMFLOAT4 color;
};

class Dx11Renderer
{
public:
    void InitDx11Renderer(HWND hWnd);
    void SetRenderTargetToScene();
    void SetRenderTargetToBackBuffer();

    Dx11Renderer() = default;

    ~Dx11Renderer();
    Dx11Renderer(const Dx11Renderer&) = delete;
    Dx11Renderer& operator=(const Dx11Renderer&) = delete;

    Camera& GetCamera();
    ID3D11Device* GetDevice() noexcept;
    ID3D11DeviceContext* GetpContext() noexcept;

    void EndFrame();

    void DrawAFrame(float deltatime, std::vector<std::unique_ptr<Instance>>& Drawables);

    void ClearBuffer(float r, float g, float b);
    void ClearSceneBuffer(float r, float g, float b);
    void ReSizeWindow(int width, int height, HWND hWnd);

    void CreateSceneResources(int width, int height);
    ID3D11ShaderResourceView* GetSceneSRV();
    ID3D11RenderTargetView* GetBackBufferRTV();
    ID3D11RenderTargetView* GetMainTarget() { return pTarget.Get(); }

    ID3D11DepthStencilView* GetDepthStencil() { return pDepthStencilView.Get(); }

    //Shadows
    void CreateShadowResources();
    void RenderShadowMap(std::vector<std::unique_ptr<Instance>>& Drawables);
    void SetShadowMapToShader();
private:
    // Alustusfunktiot
    ComPtr<ID3D11DepthStencilState> pDepthStencilState;
    void CreateDeviceAndSwapChain(int width, int height, HWND hWnd);
    void CreateViewport(int width, int height);
    void CreateDepthStencil(int width, int height);
    void CreateRenderTarget();
    void CreateConstantBuffers();
    void CompileShaders();

    Camera camera;
    float Fov = DirectX::XMConvertToRadians(90.0f);
    float zNear = 0.01f;

    ComPtr<ID3D11Device> pDevice;
    ComPtr<ID3D11DeviceContext> pContext;
    ComPtr<IDXGISwapChain> pSwap;
    ComPtr<ID3D11RenderTargetView> pTarget;
    ComPtr<ID3D11DepthStencilView> pDepthStencilView;

    ComPtr<ID3D11Buffer> pConstantBuffer;
    ComPtr<ID3D11Buffer> pColorBuffer;
    ComPtr<ID3D11Buffer> pLightingBuffer;
    ComPtr<ID3D11SamplerState> pSampler;

    ComPtr<ID3D11VertexShader> pVS;
    ComPtr<ID3D11PixelShader> pPS;
    ComPtr<ID3D11InputLayout> pLayout;

    ID3D11Buffer* pVertexBuffer = nullptr;
    ID3D11Buffer* pIndexBuffer = nullptr;
    ID3D11PixelShader* pGUIPS = nullptr;
    ID3D11Buffer* pMeshVertexBuffer = nullptr;
    ID3D11Buffer* pMeshIndexBuffer = nullptr;
    UINT MeshIndexCount = 0;

    int currentWidth = 0;
    int currentHeight = 0;

    //Viewport
    ComPtr<ID3D11Texture2D> pSceneTexture;
    ComPtr<ID3D11ShaderResourceView> pSceneSRV;
    ComPtr<ID3D11RenderTargetView> pSceneRTV;

    ComPtr<ID3D11PixelShader> pPSTexture;
    ComPtr<ID3D11PixelShader> pPSNoTexture;

    //Shadows
    ComPtr<ID3D11Texture2D> pShadowMap;
    ComPtr<ID3D11DepthStencilView> pShadowDSV;
    ComPtr<ID3D11ShaderResourceView> pShadowSRV;
    ComPtr<ID3D11RasterizerState> pShadowRasterizer;
    ComPtr<ID3D11SamplerState> pShadowSampler;
    ComPtr<ID3D11Buffer> pShadowCB;

    const UINT SHADOW_MAP_SIZE = 2048;

    struct ShadowCB
    {
        XMMATRIX lightViewProj;
        XMFLOAT3 lightPos;
        float padding;
    };
};
#endif