#include "Texture.h"
#include <iostream>
#include <vector>
#include <filesystem>
#include "UGE_ASSERTS.h"

#if VULKAN == 1
    #include <filesystem>
    #include <stdexcept>
    #include "Vulkan/VulkanHelpers.h"
    #include <Libs/STBIcons/stb_image.h>
#endif

#if DIRECTX11 == 1
    #include "Graphics/Dx11/Dx11Renderer.h"
    #include <wincodec.h>
    #include <combaseapi.h>
#endif

#include <Libs/STBIcons/stb_image.h>
//VULKAN WHERE ARE YOU

#if VULKAN == 1


namespace fs = std::filesystem;

bool Texture::LoadVK(const std::string& path, VulkanRender& vulkanrenderer)
{
    VkDevice device = vulkanrenderer.GetDevice();
    VkPhysicalDevice physicalDevice = vulkanrenderer.GetPhysicalDevice();

    UGE_VK_ASSERT(device, "Device Can't Be Null");

    if (!fs::exists(path)) {
        MakeAError("Texture Doesn't Exist! Path: " + path);
        return false;
    }

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if (!pixels) {
        MakeAError("Failed to load texture file: " + path);
        return false;
    }

    m_width = texWidth;
    m_height = texHeight;

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    std::cout << "Loading texture: " << path << " (" << texWidth << "x" << texHeight << ")" << std::endl;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    CreateBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingMemory,
        device,
        physicalDevice
    );

    void* data = nullptr;
    VkResult result = vkMapMemory(device, stagingMemory, 0, imageSize, 0, &data);

    if (result != VK_SUCCESS || !data) {
        MakeAError("Failed to map staging memory!");
        stbi_image_free(pixels);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        return false;
    }

    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingMemory);

    stbi_image_free(pixels);

    CreateImage(
        device, physicalDevice,
        texWidth, texHeight,
        m_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    VkCommandPool commandPool = vulkanrenderer.GetCommandPool();
    VkQueue graphicsQueue = vulkanrenderer.GetGraphicsQueue();

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(commandPool,device);

    TransitionImageLayout(commandBuffer, m_format,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    CopyBufferToImage(commandBuffer, stagingBuffer, texWidth, texHeight);

    TransitionImageLayout(commandBuffer, m_format,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    EndSingleTimeCommands(commandBuffer,graphicsQueue,device,commandPool);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) {
        MakeAError("Failed to create image view!");
        Cleanup(device);
        return false;
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        MakeAError("Failed to create sampler!");
        Cleanup(device);
        return false;
    }

    Loaded = true;
    std::cout << "Texture loaded successfully!" << std::endl;
    return true;
}

void Texture::CreateImage(VkDevice device, VkPhysicalDevice physicalDevice,
    uint32_t width, uint32_t height, VkFormat format,
    VkImageTiling tiling, VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &m_image) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, m_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        physicalDevice
    );

    if (vkAllocateMemory(device, &allocInfo, nullptr, &m_imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate image memory!");
    }

    vkBindImageMemory(device, m_image, m_imageMemory, 0);
}

void Texture::TransitionImageLayout(VkCommandBuffer cmd, VkFormat format,
    VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(cmd, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Texture::CopyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, uint32_t width, uint32_t height)
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(cmd, buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Texture::Cleanup(VkDevice device)
{
    if (m_sampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }
    if (m_imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, m_imageView, nullptr);
        m_imageView = VK_NULL_HANDLE;
    }
    if (m_image != VK_NULL_HANDLE) {
        vkDestroyImage(device, m_image, nullptr);
        m_image = VK_NULL_HANDLE;
    }
    if (m_imageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, m_imageMemory, nullptr);
        m_imageMemory = VK_NULL_HANDLE;
    }
}
#endif
#if DIRECTX11 == 1
ID3D11ShaderResourceView* Texture::Load(std::string path, Dx11Renderer& dx11Renderer)
{
    if (path.empty()) {
        MakeAError("Texture path is empty!");
        return nullptr;
    }

    if (!dx11Renderer.GetDevice()) {
        MakeAError("Device is null!");
        return nullptr;
    }

    std::filesystem::path fsPath(path);

    if (!std::filesystem::exists(path)) {
        MakeAError("Texture Doesnt Exist: " + path);
        return nullptr;
    }

    std::string ext = fsPath.extension().string();
    if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".bmp" && ext != ".dds") {
        MakeAError("Unsupported texture format: " + ext);
        return nullptr;
    }

    std::wstring wpath(path.begin(), path.end());

    IWICImagingFactory* factory = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
    );

    if (FAILED(hr) || !factory) {
        MakeAError("Failed to create WIC factory");
        return nullptr;
    }

    IWICBitmapDecoder* decoder = nullptr;
    hr = factory->CreateDecoderFromFilename(
        wpath.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );

    if (FAILED(hr) || !decoder) {
        MakeAError("Failed to create decoder for: " + path);
        factory->Release();
        return nullptr;
    }

    IWICBitmapFrameDecode* frame = nullptr;
    hr = decoder->GetFrame(0, &frame);

    if (FAILED(hr) || !frame) {
        MakeAError("Failed to get frame");
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    IWICFormatConverter* converter = nullptr;
    hr = factory->CreateFormatConverter(&converter);

    if (FAILED(hr) || !converter) {
        MakeAError("Failed to create converter");
        frame->Release();
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    hr = converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );

    if (FAILED(hr)) {
        MakeAError("Failed to initialize converter");
        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    UINT width, height;
    hr = frame->GetSize(&width, &height);

    if (FAILED(hr) || width == 0 || height == 0) {
        MakeAError("Invalid texture dimensions");
        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    std::vector<BYTE> pixels(width * height * 4);

    hr = converter->CopyPixels(
        nullptr,
        width * 4,
        pixels.size(),
        pixels.data()
    );

    if (FAILED(hr)) {
        MakeAError("Failed to copy pixels");
        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = pixels.data();
    data.SysMemPitch = width * 4;
    data.SysMemSlicePitch = 0;

    ID3D11Texture2D* texture = nullptr;

    UGE_ASSERT(dx11Renderer.GetDevice(), "Device Cant Be nullptr");
    hr = dx11Renderer.GetDevice()->CreateTexture2D(&desc, &data, &texture);

    if (FAILED(hr) || !texture) {
        MakeAError("Failed to create texture");
        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    ID3D11ShaderResourceView* srv = nullptr;
    hr = dx11Renderer.GetDevice()->CreateShaderResourceView(texture, nullptr, &srv);

    texture->Release();

    if (FAILED(hr) || !srv) {
        MakeAError("Failed to create shader resource view");
        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        return nullptr;
    }

    pTexture = srv;

    converter->Release();
    frame->Release();
    decoder->Release();
    factory->Release();

    Loaded = true;
    return srv;
}
#endif