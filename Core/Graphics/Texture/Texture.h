#pragma once
#include <string>
#include <filesystem>
#include "ErrorHandling/ErrorMessage.h"
#include "GLOBALS.h"
#include "Releaser.h"
#include "Debugging/Functions/UGE_ASSERTS.h"

#if DIRECTX11 == 1
    #include <d3d11.h>
    #include <wrl/client.h>

    class Dx11Renderer;

    using namespace Microsoft::WRL;
#endif

#if VULKAN == 1
    #include "Vulkan/vulkan.h"
    #include "Graphics/Vulkan/VulkanRender.h"
#endif

class Texture {
public:

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
#if DIRECTX11 == 1
    Texture() : pTexture(nullptr) {}

    ID3D11ShaderResourceView* Load(std::string path, Dx11Renderer& dx11Renderer);

    ID3D11ShaderResourceView* GetSRV() const {
        return pTexture.Get();
    }

    ID3D11ShaderResourceView* const* GetAddressOf() const {
        return pTexture.GetAddressOf();
    }

    const ComPtr<ID3D11ShaderResourceView>& GetTextureComPtr() const {
        return pTexture;
    }

    ComPtr<ID3D11ShaderResourceView>& GetTextureComPtr() {
        return pTexture;
    }
#endif

#if VULKAN == 1
    VkImageView GetImageView() const { return m_imageView; }
    VkSampler GetSampler() const { return m_sampler; }

    bool LoadVK(const std::string& path, VulkanRender& vulkanrenderer);
    void CreateImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    void TransitionImageLayout(VkCommandBuffer cmd, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, uint32_t width, uint32_t height);
    void Cleanup(VkDevice device);
#endif
    bool IsLoadedConst() const {
        return Loaded;
    }
    bool IsLoaded() {
        return Loaded;
    }
private:
    bool Loaded = false;
#if DIRECTX11 == 1
    ComPtr<ID3D11ShaderResourceView> pTexture;
#endif
#if VULKAN == 1
    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;

    VkFormat m_format = VK_FORMAT_R8G8B8A8_UNORM;
#endif
    int m_width = 0;
    int m_height = 0;
};