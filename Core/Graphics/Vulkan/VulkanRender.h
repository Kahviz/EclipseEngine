#include "GLOBALS.h"

#if VULKAN == 1
#pragma once
#include <vector>
#include <array>
#include <stdexcept>
#include <fstream>
#include <cstring>
#include <string>
#include "chrono"
#include "BoronMathLibrary.h"
#include "ScoreCounter.h"
#include <GLFW/glfw3.h>
#include "Vulkan/VulkanHelpers.h"
#include <Instances/Instance.h>
#include "Instances/Vertex.h"
#include <unordered_map>
#include "Mesh/Vulkan/MeshVulkan.h"
#include <Camera/Camera.h>
#include <minmax.h>
#include <algorithm>

//SubClasses
#include "VulkanDevice/VulkanDevice.h"
#include "VulkanInstance/VulkanInstance.h"
#include "VulkanSwapchain/VulkanSwapchain.h"
#include "VulkanCommandBuffer/VulkanCommandBuffer.h"
#include "VulkanPipeline/VulkanPipeline.h"
class Texture;

class VulkanRender {
public:
    VulkanRender() = default;
    void CreateDepthResources(uint32_t width, uint32_t height);
    bool Init(GLFWwindow* window);
    void Cleanup();
    uint32_t GetImageIndex();
    void RecordCommandBuffer(uint32_t imageIndex, bool renderImGui);
    void RecreateSwapchain();
    void CreateFramebuffers();
    void CreateImageViews();
    void CreateSwapchain();
    void createUniformBuffers();
    void ReallocateUniformBuffer(uint32_t newObjectCount);
    void createDescriptorPool();
    void UpdateDescriptorSet(const Instance* inst);
    void createDescriptorSets(const Instance* inst = nullptr);
    Matrix4x4 CreateVulkanPerspective(float fovY, float aspect, float zNear, float zFar);
    Matrix4x4 createModelMatrix(Vector3 orientation, Vector3 scale, Vector3 pos);
    void updateUniformBuffer(const Instance& inst, uint32_t objectIndex, Vector3 scale, Vector3 Orientation, Vector3 pos, Int3 color);
    bool RenderAMesh(const Instance* drawable, Vector3 Orientation, Vector3& pos, Vector3& size, Int3 color, Vector3& Velocity, bool Anchored, float Roughness, float Brightness, int Index);

    void PrintInfo();

    void RecordShadowCommandBuffer();

    void DrawFrame(float DELTATIME, std::vector<std::unique_ptr<Instance>>& Drawables);
    Camera& GetCamera();
    VkCommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    void createShadowResources();

    void createShadowRenderPass();

    void createShadowPipeline();
    //Getters
    VkDevice GetDevice() { return vkDevice.GetDevice(); };
    VkPhysicalDevice GetPhysicalDevice() { return vkDevice.GetPhysicalDevice(); };

    VkPipeline GetPipeline() { return vkPipeline.GetGraphicsPipeline(); };
    VkInstance& GetInstance() { return vkInstance.GetInstance(); };
    VkDescriptorPool& GetImGuiPool() { return imguiPool; };
    std::vector<VkCommandBuffer> GetCommandBuffers() { return vkCommandBuffer.GetCommandBuffers(); };
    uint32_t GetGraphicsFamilyIndex() { return vkDevice.GetFamilyIndex(); };
    std::vector<VkImageView> GetSwapChainImageViews() { return vkSwapchain.GetSwapchainImageViews(); };
    VkRenderPass GetRenderPass() { return renderPass; };
    VkCommandBuffer GetCurrentFrameCommandBuffer() { return vkCommandBuffer.GetCommandBuffers()[currentFrame]; };
    VkCommandPool GetCommandPool() { return vkCommandBuffer.GetCommandPool(); }
    VkQueue GetGraphicsQueue() { return vkDevice.GetGraphicsQueue(); }
private:
    struct ShadowPushConstants {
        Matrix4x4 lightSpaceMatrix;
        Matrix4x4 model;
    };
    struct ShadowDrawCommand {
        const MeshVK* mesh;
        Matrix4x4 modelMatrix;
    };
    Matrix4x4 lightSpaceMatrix;

    std::vector<ShadowDrawCommand> shadowDrawCommands;

    //SubClasses
    VulkanDevice vkDevice;
    VulkanInstance vkInstance;
    VulkanSwapchain vkSwapchain;
    VulkanCommandBuffer vkCommandBuffer;
    VulkanPipeline vkPipeline;
    Camera m_Camera;

    struct DrawCommand {
        const MeshVK* mesh;
        uint32_t objectIndex;
    };

    uint32_t CurrentimageIndex = -1;
    bool framebufferResized = false;

    int currentFrame = 0;

    std::vector<UniformBufferObject> m_UniformBuffers;
    VkBuffer m_UniformBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_UniformBufferMemory = VK_NULL_HANDLE;

    size_t m_UniformBufferSize = 0;
    uint32_t m_CurrentObjectCount = 0;

    uint32_t imageIndex = -1;
    size_t maxInstances = 100;
    size_t dynamicAlignment = -1;

    VkViewport viewport{};
    VkRect2D scissor = {};

    std::unordered_map<const Mesh*, std::unique_ptr<MeshVK>> meshCache = {};
    std::vector<DrawCommand> drawCommands = {};
    std::vector<std::unique_ptr<Instance>> DrawablesCopy = {};
    std::vector<uint32_t> drawObjectIndices = {};

    GLFWwindow* main_window = nullptr;
    void* uniformBufferMapped = nullptr;

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
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

    //Depth
    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;


    // Shadow map
    VkCommandBuffer shadowCommandBuffer = VK_NULL_HANDLE;
    VkImage shadowImage = VK_NULL_HANDLE;
    VkImageView shadowImageView = VK_NULL_HANDLE;
    VkDeviceMemory shadowImageMemory = VK_NULL_HANDLE;
    VkSampler shadowSampler = VK_NULL_HANDLE;
    VkRenderPass shadowRenderPass = VK_NULL_HANDLE;
    VkPipeline shadowPipeline = VK_NULL_HANDLE;
    VkPipelineLayout shadowPipelineLayout = VK_NULL_HANDLE;
    VkFramebuffer shadowFramebuffer = VK_NULL_HANDLE;

    const uint32_t SHADOW_MAP_SIZE = 2048;
};
#endif