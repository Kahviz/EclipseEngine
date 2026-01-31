#include "Graphics.h"
#include "Releaser.h"
#include <d3dcompiler.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include "GLOBALS.h"
#include <wrl/client.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")


void Graphics::InitGraphics(GLFWwindow* window)
{
    if (UsesDx11) {
        HWND hwnd = glfwGetWin32Window(window);
        DR = std::make_unique<Dx11Renderer>();
        DR.get()->InitDx11Renderer(hwnd);
    }

    if (UsesVulkan) {

    }
}

void Graphics::SetRenderTargetToScene() {
    if (UsesDx11) {

    }
    if (UsesVulkan) {

    }
}

void Graphics::SetRenderTargetToBackBuffer() {
    if (UsesDx11) {

    }
    if (UsesVulkan) {

    }
}

void Graphics::ReSizeWindow(int width, int height, HWND hWnd)
{
    if (UsesDx11) {
        DR.get()->ReSizeWindow(width, height, hWnd);
    }
    if (UsesVulkan) {

    }
}

void Graphics::CreateSceneResources(int width, int height) {
    if (UsesDx11) {
        DR.get()->CreateSceneResources(width, height);
    }
    if (UsesVulkan) {

    }
}

ID3D11ShaderResourceView* Graphics::GetSceneSRV() {
    if (UsesDx11) {
        return DR.get()->GetSceneSRV();
    }
    if (UsesVulkan) {

    }
}

ID3D11RenderTargetView* Graphics::GetBackBufferRTV()
{
    if (UsesDx11) {
        return DR.get()->GetBackBufferRTV();
    }
    if (UsesVulkan) {

    }
}

ID3D11RenderTargetView* Graphics::GetMainTarget()
{
    if (UsesDx11) {
        ID3D11RenderTargetView* mT = DR.get()->GetMainTarget();
        return mT;
    }
    if (UsesVulkan) {

    }
}

ID3D11DepthStencilView* Graphics::GetDepthStencil()
{
    if (UsesDx11) {
        return DR.get()->GetDepthStencil();
    }
    if (UsesVulkan) {

    }
}



Camera& Graphics::GetCamera()
{
    if (UsesDx11) {
        return DR.get()->GetCamera();
    }
    if (UsesVulkan) {

    }
}

ID3D11Device* Graphics::GetDevice() noexcept
{
    if (UsesDx11) {
        ID3D11Device* device = DR.get()->GetDevice();
        return device;
    }
    if (UsesVulkan) {

    }
}

ID3D11DeviceContext* Graphics::GetpContext() noexcept
{
    if (UsesDx11) {
        ID3D11DeviceContext* Context = DR.get()->GetpContext();
        return Context;
    }
    if (UsesVulkan) {

    }
}

void Graphics::EndFrame()
{
    if (UsesDx11) {
        DR.get()->EndFrame();
    }
    if (UsesVulkan) {

    }
}

void Graphics::DrawAFrame(float DELTATIME, std::vector<std::unique_ptr<Instance>>& Drawables) {
    if (UsesDx11) {
        DR.get()->DrawAFrame(DELTATIME,Drawables);
    }
    if (UsesVulkan) {
        VR.get()->DrawFrame();
    }
}

void Graphics::ClearBuffer(float r, float g, float b)
{
    if (UsesDx11) {
        DR.get()->ClearBuffer(r, g, b);
    }
    if (UsesVulkan) {

    }
}

void Graphics::ClearSceneBuffer(float r, float g, float b)
{
    if (UsesDx11) {
        DR.get()->ClearSceneBuffer(r, g, b);
    }
    if (UsesVulkan) {

    }
}
