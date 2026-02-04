#pragma once
#include <memory>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Mesh.h"
#include <string>
#include "Camera.h"
#include <wrl/client.h>
#include "GLFW/glfw3.h"

#include "Dx11/Dx11Renderer.h"
#include "Vulkan/VulkanRender.h"

#include "ErrorHandling/ErrorMessage.h"

class Graphics
{
public:
    Graphics() = default;
    ~Graphics() = default;  // Vain yksi destruktori!

    // Poista kopiointi
    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;

    // Muut metodit
    bool InitGraphics(GLFWwindow* window);
    void SetRenderTargetToScene();
    void SetRenderTargetToBackBuffer();

    Camera& GetCamera();

    void EndFrame();

    void DrawAFrame(float DELTATIME, std::vector<std::unique_ptr<Instance>>& Drawables);

    void ClearBuffer(float r, float g, float b);
    void ClearSceneBuffer(float r, float g, float b);
    void ReSizeWindow(int width, int height, HWND hWnd);

    void CreateSceneResources(int width, int height);

#if DIRECTX11 == 1
    ID3D11DepthStencilView* GetDepthStencil();

    ID3D11Device* GetDevice() noexcept;
    ID3D11DeviceContext* GetpContext() noexcept;

    ID3D11ShaderResourceView* GetSceneSRV();

    ID3D11RenderTargetView* GetBackBufferRTV();

    ID3D11RenderTargetView* GetMainTarget();
#endif
    

    //Getters

#if VULKAN == 1
    VkPhysicalDevice GetPhysicalDevice() const {
        return VR.get()->physicalDevice;
    }

    VkDevice GetDevice() const {
        VkDevice device = VR.get()->device;

        if (device == VK_NULL_HANDLE) {
            MakeAError("Logical device not initialized!");
        }

        return device;
    }

    auto GetCmdPool() {
        return this->VR.get()->commandPool;
    }

    auto GetGfxQueue() {
        return this->VR.get()->graphicsQueue;
    }
#endif

    std::unique_ptr<VulkanRender> VR;
    std::unique_ptr<Dx11Renderer> DR;
private:

};