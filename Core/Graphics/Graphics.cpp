#include "Graphics.h"
#include "Releaser.h"
#include <d3dcompiler.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include "GLOBALS.h"
#include <wrl/client.h>
#include "Window/Window.h"

#if DIRECTX11 == 1 
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3native.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")


bool Graphics::InitGraphics(GLFWwindow* window)
{
    #if DIRECTX11 == 1
        MakeASuccess("Inited DX11 Graphics!");

        HWND hwnd = glfwGetWin32Window(window);

        DR = std::make_unique<Dx11Renderer>();
        DR.get()->InitDx11Renderer(hwnd);

        return true;
    #endif
    #if VULKAN == 1
        MakeASuccess("Inited Vulkan Graphics!");
        VR = std::make_unique<VulkanRender>();
        return VR.get()->Init(window);
    #endif
}

void Graphics::SetRenderTargetToScene() {
    #if DIRECTX11 == 1
        DR.get()->SetRenderTargetToScene();
    #else

    #endif
}

void Graphics::RenderAMesh(
    float Deltatime,
    const Instance* drawable,
    Vector3 Orientation,
    Vector3& pos,
    Vector3& size,
    INT3 color,
    Vector3& Velocity,
    bool Anchored,
    float Roughness,
    float Brightness,
    int Index
) 
{
    #if VULKAN == 1
        VR.get()->RenderAMesh(drawable, Orientation, pos, size, color, Velocity, Anchored, Roughness, Brightness, Index);
    #endif

    #if DIRECTX11 == 1
    #endif
}
void Graphics::SetRenderTargetToBackBuffer() {
    #if DIRECTX11 == 1
        DR.get()->SetRenderTargetToBackBuffer();
    #else

    #endif
}


void Graphics::ReSizeWindow(int width, int height, Window* wnd)
{
    #if DIRECTX11 == 1
        HWND hwnd = glfwGetWin32Window(wnd->GetWindow());

        DR.get()->ReSizeWindow(width, height, hwnd);
    #else
        VR.get()->RecreateSwapchain();

    #endif
}

void Graphics::CreateSceneResources(int width, int height) {
    #if DIRECTX11 == 1
        DR.get()->CreateSceneResources(width, height);
    #else

    #endif
}

#if DIRECTX11 == 1
    ID3D11DepthStencilView* Graphics::GetDepthStencil()
    {
        return DR.get()->GetDepthStencil();
    }

    ID3D11Device* Graphics::GetDevice() noexcept
    {
        ID3D11Device* ddevice = DR.get()->GetDevice();
        return ddevice;
    }

    ID3D11DeviceContext* Graphics::GetpContext() noexcept
    {
        ID3D11DeviceContext* Context = DR.get()->GetpContext();
        return Context;
    }

    ID3D11ShaderResourceView* Graphics::GetSceneSRV() {
        return DR.get()->GetSceneSRV();
    }

    ID3D11RenderTargetView* Graphics::GetBackBufferRTV()
    {
        return DR.get()->GetBackBufferRTV();
    }

    ID3D11RenderTargetView* Graphics::GetMainTarget()
    {
        ID3D11RenderTargetView* mT = DR.get()->GetMainTarget();
        return mT;
    }
#endif


Camera& Graphics::GetCamera()
{
    #if DIRECTX11 == 1
        return DR.get()->GetCamera();
    #else
        return VR.get()->GetCamera();
    #endif
}

void Graphics::EndFrame()
{
    #if DIRECTX11 == 1
        DR.get()->EndFrame();
    #endif
}

void Graphics::DrawAFrame(float DELTATIME, std::vector<std::unique_ptr<Instance>>& Drawables) {
    #if DIRECTX11 == 1
        DR.get()->DrawAFrame(DELTATIME, Drawables);
    #else
        VR.get()->DrawFrame(DELTATIME,Drawables);
    #endif
}

void Graphics::ClearBuffer(float r, float g, float b)
{
    #if DIRECTX11 == 1
        DR.get()->ClearBuffer(r, g, b);
    #else
        
    #endif
}

void Graphics::ClearSceneBuffer(float r, float g, float b)
{
    #if DIRECTX11 == 1
        DR.get()->ClearSceneBuffer(r, g, b);
    #else

    #endif
}
