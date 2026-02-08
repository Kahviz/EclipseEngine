#pragma once
#include <vector>
#include <array>
#include <stdexcept>
#include <fstream>
#include <cstring>
#include <string>
#include "chrono"
#include "Math/UntilitedMath.h"
#include "ScoreCounter.h"
#include <GLFW/glfw3.h>
#include "Vulkan/VulkanHelpers.h"
#include <Instance.h>

struct VVertex {
    Vector3 pos;
    Vector3 color;
    
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(VVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(VVertex, color);

        return attributeDescriptions;
    }

    VVertex& operator=(const VVertex& v2) {
        return *this;
    }
};



class VulkanRender {
public:
    VulkanRender() = default;
    bool Init(GLFWwindow* window);
    void createDescriptorSetLayout();
    void Cleanup();
    void RecordCommandBuffer(uint32_t imageIndex);
    void RecreateSwapchain();
    void UpdateViewportAndScissor();
    void CreateCommandBuffers();
    void CreateFramebuffers();
    void CreateImageViews();
    void CleanupSwapchain();
    void CreateSwapchain();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void updateUniformBuffer(uint32_t objectIndex, FLOAT3 scale, FLOAT3 Orientation, FLOAT3 pos, INT3 color);
    bool RenderAMesh(const Instance* drawable, FLOAT3 Orientation, FLOAT3& pos, FLOAT3& size, INT3 color, FLOAT3& Velocity, bool Anchored, float Roughness, float Brightness, int Index);

    void DrawFrame(float DELTATIME, std::vector<std::unique_ptr<Instance>>& Drawables);

    VkCommandPool commandPool;
    VkQueue graphicsQueue;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
private:
    const uint32_t MAX_OBJECTS = 100;
    size_t maxInstances = 100;

    uint32_t imageIndex;
    int windowWidth = 800;
    int windowHeight = 800;
    int graphicsFamilyIndex = -1;
    size_t dynamicAlignment;

    std::vector<std::unique_ptr<Instance>> DrawablesCopy;
    std::vector<uint32_t> drawObjectIndices;
    std::vector<VkImage> swapchainImages;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkCommandBuffer> commandBuffers;

    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

    void* uniformBufferMapped;

    VkViewport viewport{};
    GLFWwindow* main_window;
    VkRect2D scissor{};
    VkDescriptorSetLayout descriptorSetLayout;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkExtent2D swapchainExtent;
    VkFormat swapchainImageFormat;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

    VkShaderModule vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE;

    ScoreCounter m_SC;

    std::vector<VVertex> vertices = {
        // Front face
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}}, // punainen
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}}, // vihreä
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}, // sininen
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}}, // keltainen

        // Back face
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}}, // magenta
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}}, // syaani
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}, // valkoinen
        {{-0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}}  // harmaa
    };



    const std::vector<uint16_t> indices = {
        0, 1, 2,
        2, 3, 0,

        5, 4, 7,
        7, 6, 5,

        3, 2, 6,
        6, 7, 3,

        4, 5, 1,
        1, 0, 4,

        1, 5, 6,
        6, 2, 1,

        4, 0, 3,
        3, 7, 4
    };

};