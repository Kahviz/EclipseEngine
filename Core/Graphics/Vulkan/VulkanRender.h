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
#include "Instances/Vertex.h"
#include <unordered_map>
#include "Mesh/Vulkan/MeshVulkan.h"

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
    struct DrawCommand {
        const MeshVK* mesh;
        uint32_t objectIndex;
    };

    std::vector<DrawCommand> drawCommands;
    std::unordered_map<const Mesh*, std::unique_ptr<MeshVK>> meshCache;

    const uint32_t MAX_OBJECTS = 100;
    size_t maxInstances = 100;

    uint32_t imageIndex;
    int windowWidth = 800;
    int windowHeight = 800;
    size_t graphicsFamilyIndex = -1;
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

    std::vector<Vertex> vertices = {
        {1.0f, {-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // punainen
        {1.0f, {0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // vihreä
        {1.0f, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}, // sininen
        {1.0f, {-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // keltainen

        {1.0f, {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
        {1.0f, {0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
        {1.0f, {0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
        {1.0f, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, -1.0f}}
    };



    const std::vector<uint32_t> indices = {
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