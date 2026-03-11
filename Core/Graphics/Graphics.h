#pragma once

#include <memory>
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include "Camera/Camera.h"
#include <wrl/client.h>
#include "GLFW/glfw3.h"
#include "GLOBALS.h"
class Window;

#if DIRECTX11 == 1
    #include "Dx11/Dx11Renderer.h"
#endif
#if VULKAN == 1
    #include "Vulkan/VulkanRender.h"
#endif
#include "ErrorHandling/ErrorMessage.h"

#include <vector>
#include "Instances/Instance.h"

class Graphics
{
public:
    Graphics() = default;
    ~Graphics() = default;

    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;

    bool InitGraphics(GLFWwindow* window);
    void SetRenderTargetToScene();
    void RenderAMesh(float Deltatime, const Instance* drawable, FLOAT3 Orientation, FLOAT3& pos, FLOAT3& size, INT3 color, FLOAT3& Velocity, bool Anchored, float Roughness, float Brightness, int Index);
    void SetRenderTargetToBackBuffer();

    Camera& GetCamera();

    void EndFrame();

    void DrawAFrame(float DELTATIME, std::vector<std::unique_ptr<Instance>>& Drawables);

    void ClearBuffer(float r, float g, float b);
    void ClearSceneBuffer(float r, float g, float b);

    void ReSizeWindow(int width, int height, Window* wnd);

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
        return VR.get()->GetPhysicalDevice();
    }

    VkDevice GetDevice() const {
        VkDevice device = VR.get()->GetDevice();

        if (device == VK_NULL_HANDLE) {
            MakeAError("Logical device not initialized!");
        }

        return device;
    }

    auto GetCmdPool() {
        return this->VR.get()->GetCommandPool();
    }

    auto GetGfxQueue() {
        return this->VR.get()->GetGraphicsQueue();
    }
#endif
#if VULKAN == 1
    std::unique_ptr<VulkanRender> VR;
#endif

#if DIRECTX11 == 1
    std::unique_ptr<Dx11Renderer> DR;
#endif
private:

};