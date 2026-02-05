#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include "Camera.h"
#include <wrl/client.h>
#include <Instance.h>
#include <Instances/Instances/Mesh/Mesh.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct ConstantBuffer
{
    XMMATRIX transfrom;
    XMFLOAT3 cubeColor;
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

    void DrawMesh(float deltaTime, Mesh& mesh, FLOAT3 Orientation, FLOAT3& pos, FLOAT3& size, INT3 color, FLOAT3& Velocity, bool Anchored, float Roughness, float Brightness);

    void ClearBuffer(float r, float g, float b);
    void ClearSceneBuffer(float r, float g, float b);
    void ReSizeWindow(int width, int height, HWND hWnd);

    void CreateSceneResources(int width, int height);
    ID3D11ShaderResourceView* GetSceneSRV();
    ID3D11RenderTargetView* GetBackBufferRTV();
    ID3D11RenderTargetView* GetMainTarget() { return pTarget.Get(); }

    ID3D11DepthStencilView* GetDepthStencil() { return pDepthStencilView.Get(); }
private:
    // Alustusfunktiot
    void CreateDeviceAndSwapChain(int width, int height, HWND hWnd);
    void CreateViewport(int width, int height);
    void CreateDepthStencil(int width, int height);
    void CreateRenderTarget();
    void CreateConstantBuffers();
    void CompileShaders();

    Camera camera;
    const float Gravity = 9.81f;
    float Fov = DirectX::XMConvertToRadians(90.0f);
    bool vSync = false;

    // COM smart pointerit
    ComPtr<ID3D11Device> pDevice;
    ComPtr<ID3D11DeviceContext> pContext;
    ComPtr<IDXGISwapChain> pSwap;
    ComPtr<ID3D11RenderTargetView> pTarget;
    ComPtr<ID3D11DepthStencilView> pDepthStencilView;

    ComPtr<ID3D11Buffer> pConstantBuffer;
    ComPtr<ID3D11Buffer> pColorBuffer;
    ComPtr<ID3D11Buffer> pLightingBuffer;

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
};