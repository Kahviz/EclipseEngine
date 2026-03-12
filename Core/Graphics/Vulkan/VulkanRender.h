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
#include <Instances/Instance.h>
#include "Instances/Vertex.h"
#include <unordered_map>
#include "Mesh/Vulkan/MeshVulkan.h"
#include "GLOBALS.h"
#include <Camera/Camera.h>
#include <minmax.h>
#include <algorithm>

class Texture;

class VulkanRender {
public:
    VulkanRender() = default;
    bool Init(GLFWwindow* window);
    void createDescriptorSetLayout();
    void Cleanup();
    uint32_t GetImageIndex();
    void RecordCommandBuffer(uint32_t imageIndex, bool renderImGui);
    void RecreateSwapchain();
    void UpdateViewportAndScissor();
    void CreateCommandBuffers();
    void CreateFramebuffers();
    void CreateImageViews();
    void CleanupSwapchain();
    void CreateSwapchain();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets(const Instance* inst = nullptr);
    Matrix4x4 CreateVulkanPerspective(float fovY, float aspect, float zNear, float zFar);
    void updateUniformBuffer(const Instance& inst, uint32_t objectIndex, FLOAT3 scale, FLOAT3 Orientation, FLOAT3 pos, INT3 color);
    bool RenderAMesh(const Instance* drawable, FLOAT3 Orientation, FLOAT3& pos, FLOAT3& size, INT3 color, FLOAT3& Velocity, bool Anchored, float Roughness, float Brightness, int Index);

    void DrawFrame(float DELTATIME, std::vector<std::unique_ptr<Instance>>& Drawables);
    Camera& GetCamera();
    VkCommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);


    //Getters
    VkPipeline GetPipeline() { return graphicsPipeline; };
    VkInstance& GetVulkanInstance() { return instance; };
    VkDescriptorPool& GetImGuiPool() { return imguiPool; };
    std::vector<VkCommandBuffer> GetCommandBuffers() { return commandBuffers; };
    size_t GetGraphicsFamilyIndex() { return graphicsFamilyIndex; };
    std::vector<VkImageView> GetSwapChainImageViews() { return swapchainImageViews; };
    VkRenderPass GetRenderPass() { return renderPass; };  
    VkCommandBuffer GetCurrentFrameCommandBuffer() { return commandBuffers[currentFrame]; };
    VkCommandPool GetCommandPool() { return commandPool; }
    VkDevice GetDevice() { return device; };
    VkQueue GetGraphicsQueue() { return graphicsQueue; };
    VkPhysicalDevice GetPhysicalDevice() { return physicalDevice; };
private:
    Camera m_Camera;
    ScoreCounter m_SC;
    Texture* defaultTexture;
    struct DrawCommand {
        const MeshVK* mesh;
        uint32_t objectIndex;
    };

    uint32_t CurrentimageIndex = -1;
    bool framebufferResized = false;

    int currentFrame = 0;

    const uint32_t MAX_OBJECTS = 100;
    uint32_t imageIndex = -1;
    size_t maxInstances = 100;
    size_t graphicsFamilyIndex = -1;
    size_t dynamicAlignment = -1;

    VkExtent2D swapchainExtent = {};
    VkFormat swapchainImageFormat = {};
    VkViewport viewport{};
    VkRect2D scissor = {};

    std::unordered_map<const Mesh*, std::unique_ptr<MeshVK>> meshCache = {};
    std::vector<DrawCommand> drawCommands = {};
    std::vector<VkCommandBuffer> commandBuffers = {};
    std::vector<std::unique_ptr<Instance>> DrawablesCopy = {};
    std::vector<uint32_t> drawObjectIndices = {};
    std::vector<VkImage> swapchainImages = {};
    std::vector<VkFramebuffer> swapchainFramebuffers = {};
    std::vector<VkImageView> swapchainImageViews = {};

    GLFWwindow* main_window = nullptr;
    void* uniformBufferMapped = nullptr;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkBuffer uniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory uniformBufferMemory = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkDescriptorPool imguiPool = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkFence inFlightFence = VK_NULL_HANDLE;
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkShaderModule vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
};