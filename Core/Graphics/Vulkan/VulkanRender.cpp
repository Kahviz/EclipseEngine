#include "GLOBALS.h"

#if VULKAN == 1

#include "VulkanRender.h"

#include <cstdint>
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include "ErrorHandling/ErrorMessage.h"
#include "Mesh/Vulkan/MeshVulkan.h"
#include <CameraControl.h>
#include "imgui.h"
#include <imgui_impl_vulkan.h>
#include "Texture.h"

bool VulkanRender::Init(GLFWwindow* window)
{
    float AspectX = (float)screen_width;
    float AspectY = (float)screen_height;
    float Aspect = AspectX / AspectY;

    m_Camera.SetProjectionValues(FOV, Aspect, 0.0f, 1000.0f);

    #ifdef _DEBUG
        std::cout << "Vulkan Init Started\n";
    #endif
    
    if (!vkInstance.Init()) {
        MakeAError("A Unexpected error happened on vkInstance.Init");
        return false;
    }

    if (!vkDevice.Init(window, vkInstance.GetInstance())) {
        MakeAError("A Unexpected error happened on vkDevice.Init");
        return false;
    }

    if (!vkCommandBuffer.CreateCommandPool(vkDevice.GetDevice(), vkDevice.GetFamilyIndex())) {
        MakeAError("A Unexpected error happened on vkDevice.CreateCommandPool");
        return false;
    }

    VkPhysicalDeviceProperties selectedProps;
    vkGetPhysicalDeviceProperties(vkDevice.GetPhysicalDevice(), &selectedProps);

    #ifdef _DEBUG
        std::cout << "Selected GPU: " << selectedProps.deviceName << "\n";
    #endif // _DEBUG

   
    uint32_t formatCount = 0;

    vkGetPhysicalDeviceSurfaceFormatsKHR(vkDevice.GetPhysicalDevice(), vkDevice.GetSurface(), &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkDevice.GetPhysicalDevice(), vkDevice.GetSurface(), &formatCount, formats.data());
     
    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = availableFormat;
            break;
        }
    }

    if (!vkSwapchain.Init(vkDevice.GetDevice(),vkDevice.GetPhysicalDevice(), vkDevice.GetSurface())) {
        MakeAError("A Unexpected error happened on vkSwapchain.Init");
    }

    std::array<VkAttachmentDescription, 2> attachments = {};
    
    // Color attachment
    attachments[0].format = surfaceFormat.format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Depth attachment
    depthFormat = FindDepthFormat(vkDevice.GetPhysicalDevice());
    attachments[1].format = depthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Attachment references
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    //Create DepthRecources
    CreateDepthResources(vkSwapchain.GetSwapchainExtent().width, vkSwapchain.GetSwapchainExtent().height);

    // Render pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(vkDevice.GetDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        MakeAError("Failed to create render pass");
        return false;
    }
    MakeASuccess("Render pass created");

    depthFormat = FindDepthFormat(vkDevice.GetPhysicalDevice());

    if (!vkPipeline.Init(vkDevice.GetDevice(), renderPass)) {
        MakeAError("A Unexpected error happened on vkPipeline.Init");
    }

    MakeASuccess("Framebuffers created");

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(vkDevice.GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(vkDevice.GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(vkDevice.GetDevice(), &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
        MakeAError("Failed to create synchronization objects!");
        return false;
    }

    MakeASuccess("Synchronization objects created");

    uint32_t framebufferCount = static_cast<uint32_t>(vkSwapchain.GetSwapchainFramebuffers().size());

    if (!vkCommandBuffer.AllocateCommandBuffers(vkDevice.GetDevice(), framebufferCount)) {
        MakeAError("Failed to allocate command buffers!");
        return false;
    }

    vkSwapchain.GetSwapchainExtent() = vkSwapchain.ChooseSwapchainExtent(vkDevice.GetPhysicalDevice(),vkDevice.GetSurface());
    vkSwapchain.GetSwapchainImageFormat() = surfaceFormat.format;

    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(vkDevice.GetPhysicalDevice(), &props);

    size_t minAlignment = props.limits.minUniformBufferOffsetAlignment;

    size_t alignedUBOSize =
        sizeof(UniformBufferObject);

    if (minAlignment > 0) {
        alignedUBOSize =
            (alignedUBOSize + minAlignment - 1) & ~(minAlignment - 1);
    }

    dynamicAlignment = alignedUBOSize;


    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    if (vkCreateDescriptorPool(vkDevice.GetDevice(), &pool_info, nullptr, &imguiPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create ImGui descriptor pool");
    }

    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets(nullptr);

    createShadowRenderPass();
    createShadowResources();
    createShadowPipeline();

    //InitEnd
    MakeASuccess("No Fatal Errors in Vulkan Initing :D-<");
    return true;
}

void VulkanRender::CreateDepthResources(uint32_t width, uint32_t height) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(vkDevice.GetDevice(), &imageInfo, nullptr, &depthImage) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create depth image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vkDevice.GetDevice(), depthImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vkDevice.GetPhysicalDevice()
    );

    if (vkAllocateMemory(vkDevice.GetDevice(), &allocInfo, nullptr, &depthImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate depth image memory!");
    }

    vkBindImageMemory(vkDevice.GetDevice(), depthImage, depthImageMemory, 0);

    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = depthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(vkDevice.GetDevice(), &viewInfo, nullptr, &depthImageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create depth image view!");
    }
}

void VulkanRender::Cleanup()
{
    MakeAInfo("Starting Vulkan Cleanup");

    if (vkDevice.GetDevice() != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(vkDevice.GetDevice());
    }

    if (imguiPool != VK_NULL_HANDLE) {
        ImGui_ImplVulkan_Shutdown();
        vkDestroyDescriptorPool(vkDevice.GetDevice(), imguiPool, nullptr);
        imguiPool = VK_NULL_HANDLE;
    }

    meshCache.clear();

    if (shadowFramebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(vkDevice.GetDevice(), shadowFramebuffer, nullptr);
        shadowFramebuffer = VK_NULL_HANDLE;
    }
    if (shadowSampler != VK_NULL_HANDLE) {
        vkDestroySampler(vkDevice.GetDevice(), shadowSampler, nullptr);
        shadowSampler = VK_NULL_HANDLE;
    }
    if (shadowImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(vkDevice.GetDevice(), shadowImageView, nullptr);
        shadowImageView = VK_NULL_HANDLE;
    }
    if (shadowImage != VK_NULL_HANDLE) {
        vkDestroyImage(vkDevice.GetDevice(), shadowImage, nullptr);
        shadowImage = VK_NULL_HANDLE;
    }
    if (shadowImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(vkDevice.GetDevice(), shadowImageMemory, nullptr);
        shadowImageMemory = VK_NULL_HANDLE;
    }
    if (!vkSwapchain.CleanupSwapchain(vkDevice.GetDevice(), vkCommandBuffer.GetCommandPool(), vkCommandBuffer.GetCommandBuffers())) {
        MakeAError("A Error happened in vkSwapchain.CleanupSwapchain");
    }

    if (indexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(vkDevice.GetDevice(), indexBuffer, nullptr);
        indexBuffer = VK_NULL_HANDLE;
    }
    if (indexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(vkDevice.GetDevice(), indexBufferMemory, nullptr);
        indexBufferMemory = VK_NULL_HANDLE;
    }

    if (vkPipeline.GetVertShaderModule() != VK_NULL_HANDLE) {
        vkDestroyShaderModule(vkDevice.GetDevice(), vkPipeline.GetVertShaderModule(), nullptr);
    }
    if (vkPipeline.GetFragShaderModule() != VK_NULL_HANDLE) {
        vkDestroyShaderModule(vkDevice.GetDevice(), vkPipeline.GetFragShaderModule(), nullptr);
    }

    if (depthImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(vkDevice.GetDevice(), depthImageView, nullptr);
        depthImageView = VK_NULL_HANDLE;
    }
    if (depthImage != VK_NULL_HANDLE) {
        vkDestroyImage(vkDevice.GetDevice(), depthImage, nullptr);
        depthImage = VK_NULL_HANDLE;
    }
    if (depthImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(vkDevice.GetDevice(), depthImageMemory, nullptr);
        depthImageMemory = VK_NULL_HANDLE;
    }

    if (vkDevice.GetSurface() != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(vkInstance.GetInstance(), vkDevice.GetSurface(), nullptr);
    }

    if (vkDevice.GetDevice() != VK_NULL_HANDLE) {
        vkDestroyDevice(vkDevice.GetDevice(), nullptr);
    }

    if (vkInstance.GetInstance() != VK_NULL_HANDLE) {
        vkDestroyInstance(vkInstance.GetInstance(), nullptr);
    }
    MakeAInfo("Cleaned up successfully!");
}

uint32_t VulkanRender::GetImageIndex() {
    return CurrentimageIndex;
}

void VulkanRender::RecordCommandBuffer(uint32_t imageIndex, bool renderImGui)
{
    CurrentimageIndex = imageIndex;
    VkCommandBuffer cmd = vkCommandBuffer.GetCommandBuffers()[imageIndex];
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)screen_width;
    viewport.height = (float)screen_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { (uint32_t)screen_width, (uint32_t)screen_height };
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.208f, 0.51f, 0.741f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };


    VkRenderPassBeginInfo rp{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    rp.renderPass = renderPass;
    rp.framebuffer = vkSwapchain.GetSwapchainFramebuffers()[imageIndex];
    rp.renderArea.extent = vkSwapchain.GetSwapchainExtent();
    rp.clearValueCount = static_cast<uint32_t>(clearValues.size());
    rp.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline.GetGraphicsPipeline());

    for (const auto& drawCmd : drawCommands)
    {
        uint32_t dynamicOffset = drawCmd.objectIndex * dynamicAlignment;
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vkPipeline.GetPipelineLayout(),
            0, 1, &descriptorSet,
            1, &dynamicOffset
        );
        drawCmd.mesh->Draw(cmd);
    }

    if (renderImGui && ImGui::GetDrawData()) {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    }

    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);
}

void VulkanRender::RecreateSwapchain() {
    vkDeviceWaitIdle(vkDevice.GetDevice());

    vkSwapchain.CleanupSwapchain(vkDevice.GetDevice(), vkCommandBuffer.GetCommandPool(), vkCommandBuffer.GetCommandBuffers());

    CreateSwapchain();
    CreateImageViews();
    CreateDepthResources(vkSwapchain.GetSwapchainExtent().width, vkSwapchain.GetSwapchainExtent().height);
    CreateFramebuffers();
    uint32_t framebufferCount = static_cast<uint32_t>(vkSwapchain.GetSwapchainFramebuffers().size());

    if (!vkCommandBuffer.AllocateCommandBuffers(vkDevice.GetDevice(), framebufferCount)) {
        MakeAError("Failed to allocate command buffers at RecreateSwapchain!");
    }

    for (size_t i = 0; i < vkCommandBuffer.GetCommandBuffers().size(); i++) {
        RecordCommandBuffer(static_cast<uint32_t>(i),false);
    }
}

void VulkanRender::CreateFramebuffers() {
    vkSwapchain.GetSwapchainFramebuffers().resize(vkSwapchain.GetSwapchainImageViews().size());

    for (size_t i = 0; i < vkSwapchain.GetSwapchainImageViews().size(); i++) {
        std::array<VkImageView, 2> attachments = {
            vkSwapchain.GetSwapchainImageViews()[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = vkSwapchain.GetSwapchainExtent().width;
        framebufferInfo.height = vkSwapchain.GetSwapchainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vkDevice.GetDevice(), &framebufferInfo, nullptr, &vkSwapchain.GetSwapchainFramebuffers()[i]) != VK_SUCCESS) {
            MakeAError("Failed to create framebuffer!");
        }
    }
}

void VulkanRender::CreateImageViews() {
    vkSwapchain.GetSwapchainImageViews().resize(vkSwapchain.GetSwapchainImages().size());

    for (size_t i = 0; i < vkSwapchain.GetSwapchainImages().size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = vkSwapchain.GetSwapchainImages()[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = vkSwapchain.GetSwapchainImageFormat();
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vkDevice.GetDevice(), &viewInfo, nullptr, &vkSwapchain.GetSwapchainImageViews()[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image views!");
        }
    }
}

void VulkanRender::CreateSwapchain() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkDevice.GetPhysicalDevice(), vkDevice.GetSurface(), &surfaceCapabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkDevice.GetPhysicalDevice(), vkDevice.GetSurface(), &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkDevice.GetPhysicalDevice(), vkDevice.GetSurface(), &formatCount, formats.data());

    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = availableFormat;
            break;
        }
    }

    VkPresentModeKHR presentMode;
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkDevice.GetPhysicalDevice(), vkDevice.GetSurface(), &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkDevice.GetPhysicalDevice(), vkDevice.GetSurface(), &presentModeCount, presentModes.data());

    if (vSync) {
        presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }
    else {
        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

        for (const auto& mode : presentModes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                presentMode = mode;
                break;
            }
        }
    }

    VkExtent2D extent = surfaceCapabilities.currentExtent;
    if (extent.width == UINT32_MAX || extent.width == 0) {
        // width
        uint32_t minWidth = surfaceCapabilities.minImageExtent.width;
        uint32_t maxWidth = surfaceCapabilities.maxImageExtent.width;
        uint32_t targetWidth = static_cast<uint32_t>(screen_width);

        if (targetWidth < minWidth)
            extent.width = minWidth;
        else if (targetWidth > maxWidth)
            extent.width = maxWidth;
        else
            extent.width = targetWidth;

        uint32_t minHeight = surfaceCapabilities.minImageExtent.height;
        uint32_t maxHeight = surfaceCapabilities.maxImageExtent.height;
        uint32_t targetHeight = static_cast<uint32_t>(screen_height);

        if (targetHeight < minHeight)
            extent.height = minHeight;
        else if (targetHeight > maxHeight)
            extent.height = maxHeight;
        else
            extent.height = targetHeight;
    }

    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
        imageCount = surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = vkDevice.GetSurface();
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(vkDevice.GetDevice(), &swapchainCreateInfo, nullptr, &vkSwapchain.GetSwapchain()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swapchain!");
    }

    vkSwapchain.GetSwapchainExtent() = extent;
    vkSwapchain.GetSwapchainImageFormat() = surfaceFormat.format;

    // Hae swapchain kuvat
    uint32_t swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(vkDevice.GetDevice(), vkSwapchain.GetSwapchain(), &swapchainImageCount, nullptr);
    vkSwapchain.GetSwapchainImages().resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(vkDevice.GetDevice(), vkSwapchain.GetSwapchain(), &swapchainImageCount, vkSwapchain.GetSwapchainImages().data());
}

void VulkanRender::createUniformBuffers() {
    m_CurrentObjectCount = 100;
    ReallocateUniformBuffer(m_CurrentObjectCount);
}

void VulkanRender::ReallocateUniformBuffer(uint32_t newObjectCount) {
    if (m_UniformBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(vkDevice.GetDevice(), m_UniformBuffer, nullptr);
        vkFreeMemory(vkDevice.GetDevice(), m_UniformBufferMemory, nullptr);
    }

    m_UniformBufferSize = dynamicAlignment * newObjectCount;
    m_CurrentObjectCount = newObjectCount;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = m_UniformBufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vkDevice.GetDevice(), &bufferInfo, nullptr, &m_UniformBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create uniform buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vkDevice.GetDevice(), m_UniformBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vkDevice.GetPhysicalDevice()
    );

    if (vkAllocateMemory(vkDevice.GetDevice(), &allocInfo, nullptr, &m_UniformBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate uniform buffer memory!");
    }

    vkBindBufferMemory(vkDevice.GetDevice(), m_UniformBuffer, m_UniformBufferMemory, 0);
    vkMapMemory(vkDevice.GetDevice(), m_UniformBufferMemory, 0, m_UniformBufferSize, 0, &uniformBufferMapped);
}

void VulkanRender::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[0].descriptorCount = 1;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(vkDevice.GetDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void VulkanRender::UpdateDescriptorSet(const Instance* inst) {
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(vkDevice.GetDevice(), descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }

    createDescriptorPool();
    createDescriptorSets(inst);
}

void VulkanRender::createDescriptorSets(const Instance* inst) {
    const Texture* texture = (inst != nullptr) ? inst->GetConstTexture() : nullptr;

    VkDescriptorSetLayout layouts[] = { vkPipeline.GetDescriptorSetLayout() };

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(vkDevice.GetDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_UniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = dynamicAlignment;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[0].pImageInfo = nullptr;
    descriptorWrites[0].pTexelBufferView = nullptr;

    VkDescriptorImageInfo imageInfo{};
    if (texture != nullptr && texture->IsLoadedConst()) {
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture->GetImageView();
        imageInfo.sampler = texture->GetSampler();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;
        descriptorWrites[1].pBufferInfo = nullptr;
        descriptorWrites[1].pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(vkDevice.GetDevice(), 2, descriptorWrites.data(), 0, nullptr); // 2 bindings
    }
    else {
        vkUpdateDescriptorSets(vkDevice.GetDevice(), 1, &descriptorWrites[0], 0, nullptr); //only 1 binding
    }
}

Matrix4x4 VulkanRender::CreateVulkanPerspective(float fovY, float aspect, float zNear, float zFar) {
    Matrix4x4 result(0.0f);
    float f = 1.0f / tanf(fovY * 0.5f);
    float rangeInv = 1.0f / (zFar - zNear);

    result(0, 0) = f / aspect;
    result(1, 1) = -f;
    result(2, 2) = zFar * rangeInv;
    result(2, 3) = 1.0f;
    result(3, 2) = -(zFar * zNear) * rangeInv;

    return result;
}
void VulkanRender::updateUniformBuffer(
    const Instance& inst,
    uint32_t objectIndex,
    Vector3 scale,
    Vector3 Orientation,
    Vector3 pos,
    Int3 color
)
{
    if (objectIndex >= m_CurrentObjectCount) {
        uint32_t newSize = max(m_CurrentObjectCount * 2, objectIndex + 1);
        ReallocateUniformBuffer(newSize);

        UpdateDescriptorSet(&inst);
    }

    UniformBufferObject ubo{};

    float angle = Orientation.y();
    float cosA = cosf(angle);
    float sinA = sinf(angle);

    ubo.model.setIdentity();
    ubo.model(0, 0) = cosA * scale.x();
    ubo.model(0, 2) = sinA * scale.x();
    ubo.model(1, 1) = scale.y();
    ubo.model(2, 0) = -sinA * scale.z();
    ubo.model(2, 2) = cosA * scale.z();
    ubo.model(3, 0) = pos.x();
    ubo.model(3, 1) = pos.y();
    ubo.model(3, 2) = pos.z();
    ubo.model(3, 3) = 1.0f;

    //Color
    ubo.color = GPUVector3(
        color.x() / 255.0f,
        color.y() / 255.0f,
        color.z() / 255.0f
    );

    const Texture* tex = inst.GetConstTexture();

    if (tex != nullptr && tex->IsLoadedConst()) {
        ubo.UsesTexture = 1.0f;
    }
    else {
        ubo.UsesTexture = 0.0f;
    }

    ubo.view = m_Camera.GetViewMatrix().transposed();

    float fovY = 45.0f * PI / 180.0f;
    float aspect = (float)screen_width / (float)screen_height;
    ubo.proj = CreateVulkanPerspective(fovY, aspect, 0.1f, zFar);

    uint8_t* dst = (uint8_t*)uniformBufferMapped + objectIndex * dynamicAlignment;
    memcpy(dst, &ubo, sizeof(ubo));
}

bool VulkanRender::RenderAMesh(
    const Instance* drawable,
    Vector3 Orientation,
    Vector3& pos,
    Vector3& size,
    Int3 color,
    Vector3& Velocity,
    bool Anchored,
    float Roughness,
    float Brightness,
    int Index
)
{
    
#if VULKAN == 1
    if (drawable == nullptr) {
        return false;
    }

    const MeshVK* meshVK = &drawable->OBJmesh.VM;

    updateUniformBuffer(*drawable ,Index, size, Orientation, pos, color);

    DrawCommand cmd;
    cmd.mesh = meshVK;
    cmd.objectIndex = Index;
    drawCommands.push_back(cmd);

    return true;
#else
    return false;
#endif // VULKAN == 1
}

void VulkanRender::PrintInfo() {
    /*
    VkImage shadowImage = VK_NULL_HANDLE;
    VkImageView shadowImageView = VK_NULL_HANDLE;
    VkDeviceMemory shadowImageMemory = VK_NULL_HANDLE;
    VkSampler shadowSampler = VK_NULL_HANDLE;
    VkRenderPass shadowRenderPass = VK_NULL_HANDLE;
    VkPipeline shadowPipeline = VK_NULL_HANDLE;
    VkPipelineLayout shadowPipelineLayout = VK_NULL_HANDLE;
    VkFramebuffer shadowFramebuffer = VK_NULL_HANDLE;

    VkCommandBuffer shadowCommandBuffer = VK_NULL_HANDLE;
    */

    if (shadowImage == VK_NULL_HANDLE) {
        MakeAError("shadowImage is VK_NULL_HANDLE");
    }
    if (shadowImageView == VK_NULL_HANDLE) {
        MakeAError("shadowImageView is VK_NULL_HANDLE");
    }
    if (shadowImageMemory == VK_NULL_HANDLE) {
        MakeAError("shadowImageMemory is VK_NULL_HANDLE");
    }
    if (shadowSampler == VK_NULL_HANDLE) {
        MakeAError("shadowSampler is VK_NULL_HANDLE");
    }
    if (shadowRenderPass == VK_NULL_HANDLE) {
        MakeAError("shadowRenderPass is VK_NULL_HANDLE");
    }
    if (shadowPipeline == VK_NULL_HANDLE) {
        MakeAError("shadowPipeline is VK_NULL_HANDLE");
    }
    if (shadowPipelineLayout == VK_NULL_HANDLE) {
        MakeAError("shadowPipelineLayout is VK_NULL_HANDLE");
    }
    if (shadowFramebuffer == VK_NULL_HANDLE) {
        MakeAError("shadowFramebuffer is VK_NULL_HANDLE");
    }
    if (shadowCommandBuffer == VK_NULL_HANDLE) {
        MakeAError("shadowCommandBuffer is VK_NULL_HANDLE");
    }
    MakeAInfo("Checked all the shadowResources");
}

void VulkanRender::DrawFrame(float DELTATIME, std::vector<std::unique_ptr<Instance>>& Drawables)
{
    static int frames = 0;
    frames++;

    if (frames == 500) {
        frames = 0;

        PrintInfo();
    }

    vkWaitForFences(vkDevice.GetDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(vkDevice.GetDevice(), 1, &inFlightFence);

    VkResult result = vkAcquireNextImageKHR(
        vkDevice.GetDevice(), vkSwapchain.GetSwapchain(), UINT64_MAX,
        imageAvailableSemaphore,
        VK_NULL_HANDLE,
        &imageIndex
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        drawCommands.clear();
        RecreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        MakeAError("Failed to acquire swapchain image");
        return;
    }

    if (framebufferResized) {
        framebufferResized = false;
        drawCommands.clear();
        RecreateSwapchain();
        return;
    }
    bool RenderImGui = true;

    RecordCommandBuffer(imageIndex,RenderImGui);

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkCommandBuffer.GetCommandBuffers()[imageIndex];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(vkDevice.GetGraphicsQueue(), 1, &submitInfo, inFlightFence);

    VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vkSwapchain.GetSwapchain();
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(vkDevice.GetGraphicsQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        drawCommands.clear();
        RecreateSwapchain();
    }
    else if (result != VK_SUCCESS) {
        MakeAError("Failed to present swapchain image");
    }

    drawCommands.clear();
}
Camera& VulkanRender::GetCamera()
{
    return m_Camera;
}

VkCommandBuffer VulkanRender::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vkCommandBuffer.GetCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(vkDevice.GetDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
        MakeAError("Failed to allocate command buffer for single time commands");
        return VK_NULL_HANDLE;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        MakeAError("Failed to begin command buffer");
        return VK_NULL_HANDLE;
    }

    return commandBuffer;
}

void VulkanRender::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    if (commandBuffer == VK_NULL_HANDLE) return;

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        MakeAError("Failed to end command buffer");
        return;
    }
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (vkQueueSubmit(vkDevice.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        MakeAError("Failed to submit command buffer");
    }

    vkQueueWaitIdle(vkDevice.GetGraphicsQueue());

    vkFreeCommandBuffers(vkDevice.GetDevice(), vkCommandBuffer.GetCommandPool(), 1, &commandBuffer);
}

//Shadows
void VulkanRender::createShadowResources()
{
    BGE_VK_ASSERT(vkDevice.GetPhysicalDevice(), "Physical device is VK_NULL_HANDLE!");

    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(vkDevice.GetPhysicalDevice(), VK_FORMAT_D32_SFLOAT, &formatProps);

    if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
        MakeAError("VK_FORMAT_D32_SFLOAT does not support depth attachment!");
    }

    VkImageCreateInfo imageinfo{};
    imageinfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageinfo.imageType = VK_IMAGE_TYPE_2D;
    imageinfo.extent.width = SHADOW_MAP_SIZE;
    imageinfo.extent.height = SHADOW_MAP_SIZE;
    imageinfo.extent.depth = 1;
    imageinfo.mipLevels = 1;
    imageinfo.arrayLayers = 1;
    imageinfo.format = VK_FORMAT_D32_SFLOAT;
    imageinfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageinfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageinfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    BGE_ASSERT_VKRESULT(vkCreateImage(vkDevice.GetDevice(), &imageinfo, nullptr, &shadowImage),"Failed to create shadowImage");

    if (shadowImage == VK_NULL_HANDLE) {
        MakeAError("shadowImage is NULL even though creation succeeded!");
        return;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vkDevice.GetDevice(), shadowImage, &memRequirements);

    if (memRequirements.size == 0) {
        MakeAError("Memory size is 0");
        MakeAError("This suggests a driver bug or invalid image parameters");
        return;
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vkDevice.GetPhysicalDevice()
    );

    BGE_ASSERT_VKRESULT(vkAllocateMemory(vkDevice.GetDevice(), &allocInfo, nullptr, &shadowImageMemory), "Failed to allocate memory for shadowImageMemory");

    if (vkBindImageMemory(vkDevice.GetDevice(), shadowImage, shadowImageMemory, 0) != VK_SUCCESS) {
        MakeAError("Failed to bind memory for shadowImageMemory");
    }

    MakeASuccess("Allocated shadowImageMemory");

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = shadowImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_D32_SFLOAT;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    BGE_ASSERT_VKRESULT(vkCreateImageView(vkDevice.GetDevice(), &viewInfo, nullptr, &shadowImageView), "Failed to create shadowImageView");

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = VK_COMPARE_OP_LESS;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    
    if (vkCreateSampler(vkDevice.GetDevice(), &samplerInfo, nullptr, &shadowSampler) != VK_SUCCESS) {
        MakeAError("Failed to create shadow sampler!");
    }

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = shadowRenderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &shadowImageView;
    framebufferInfo.width = SHADOW_MAP_SIZE;
    framebufferInfo.height = SHADOW_MAP_SIZE;
    framebufferInfo.layers = 1;
    
    BGE_ASSERT_VKRESULT(vkCreateFramebuffer(vkDevice.GetDevice(), &framebufferInfo, nullptr, &shadowFramebuffer), "Failed to create shadow framebuffer");

    VkCommandBufferAllocateInfo commandAllocInfo{};
    commandAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandAllocInfo.commandPool = vkCommandBuffer.GetCommandPool();
    commandAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandAllocInfo.commandBufferCount = 1;

    BGE_ASSERT_VKRESULT(vkAllocateCommandBuffers(vkDevice.GetDevice(), &commandAllocInfo, &shadowCommandBuffer),"Failed to allocate ShadowCommandBuffer");
}

void VulkanRender::createShadowRenderPass() {
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;
    subpass.pColorAttachments = nullptr;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &depthAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(vkDevice.GetDevice(), &renderPassInfo, nullptr, &shadowRenderPass) != VK_SUCCESS) {
        MakeAError("Failed to create shadowRenderpass");
    }
    else {
        MakeAInfo("shadowRenderpass created!");
    }
}
void VulkanRender::createShadowPipeline() {
    std::string Shaders = std::string(PROJECT_DIR) + "Core/Shaders/";
    auto vertShaderCode = ReadFile(Shaders + "shadow_vertex.spv");
    VkShaderModule vertShaderModule = CreateShaderModule(vkDevice.GetDevice(), vertShaderCode);

    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertShaderModule;
    vertStage.pName = "main";

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;

    if (vkCreatePipelineLayout(vkDevice.GetDevice(), &pipelineLayoutInfo, nullptr, &shadowPipelineLayout) != VK_SUCCESS) {
        MakeAError("Failed to create shadow pipeline layout!");
        return;
    }

    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(float) * 3;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attribute{};
    attribute.binding = 0;
    attribute.location = 0;
    attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute.offset = 0;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &binding;
    vertexInputInfo.vertexAttributeDescriptionCount = 1;
    vertexInputInfo.pVertexAttributeDescriptions = &attribute;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)SHADOW_MAP_SIZE;
    viewport.height = (float)SHADOW_MAP_SIZE;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { SHADOW_MAP_SIZE, SHADOW_MAP_SIZE };

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_TRUE;
    rasterizer.depthBiasConstantFactor = 1.25f;
    rasterizer.depthBiasSlopeFactor = 1.75f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 0;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 1;
    pipelineInfo.pStages = &vertStage;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = shadowPipelineLayout;
    pipelineInfo.renderPass = shadowRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(vkDevice.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &shadowPipeline) != VK_SUCCESS) {
        MakeAError("Failed to create shadow pipeline!");
    }

    vkDestroyShaderModule(vkDevice.GetDevice(), vertShaderModule, nullptr);

    MakeASuccess("Shadow pipeline created!");
}
#endif