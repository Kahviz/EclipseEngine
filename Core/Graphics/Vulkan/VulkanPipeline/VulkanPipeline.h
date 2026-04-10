#pragma once
#include "GLOBALS.h"

#if VULKAN == 1
#include "ErrorHandling/Errormessage.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <Graphics/Vulkan/ScoreCounter.h>
#include "Vulkan/VulkanHelpers.h"
#include "Instances/Vertex.h"

class VulkanPipeline {
public:
    bool Init(VkDevice device, VkRenderPass renderPass);

    void createDescriptorSetLayout(VkDevice device);

    //Getters
    VkPipeline& GetGraphicsPipeline() { return graphicsPipeline; }
    const VkPipeline& GetGraphicsPipeline() const { return graphicsPipeline; }

    VkPipelineLayout& GetPipelineLayout() { return pipelineLayout; }
    const VkPipelineLayout& GetPipelineLayout() const { return pipelineLayout; }

    VkShaderModule& GetVertShaderModule() { return vertShaderModule; }
    const VkShaderModule& GetVertShaderModule() const { return vertShaderModule; }

    VkShaderModule& GetFragShaderModule() { return fragShaderModule; }
    const VkShaderModule& GetFragShaderModule() const { return fragShaderModule; }

    VkDescriptorSetLayout& GetDescriptorSetLayout() { return descriptorSetLayout; }
    const VkDescriptorSetLayout& GetDescriptorSetLayout() const { return descriptorSetLayout; }
private:
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkShaderModule vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
};
#endif