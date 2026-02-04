#pragma once
#include <vulkan/vulkan.h>

class MeshVK
{
public:
    void Load(
        const std::string& file,
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandPool commandPool,
        VkQueue graphicsQueue
    );

    void Draw(VkCommandBuffer cmd) const;

    const std::vector<uint32_t>& GetIndices() const {
        return indices;
    }

    const std::vector<Vertex>& GetVertices() const {
        return verts;
    }
private:
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexMemory = VK_NULL_HANDLE;

    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexMemory = VK_NULL_HANDLE;

    uint32_t indexCount = 0;

    std::vector<Vertex> verts;
    std::vector<uint32_t> indices;
};
