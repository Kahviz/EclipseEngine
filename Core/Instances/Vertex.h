#pragma once
#include <array>
#include "BoronMathLibrary.h"
#include "Vulkan/vulkan.h"

struct Vertex
{
    float brightness = 1.0f;
    GPUVector3 pos = { 0,0,0 };
    GPUVector3 color = { 0,0,0 };
    GPUVector3 normal = { 0,0,0 };
    GPUVector2 uv = { 0,0 };
    Vertex() = default;

    Vertex(float b, const GPUVector3& p, const GPUVector3& c, const GPUVector3& n)
        : brightness(b), pos(p), color(c), normal(n), uv{ 0.0f, 0.0f } {
    }

    Vertex(float b, const GPUVector3& p, const GPUVector3& c, const GPUVector3& n, const GPUVector2& uv_coords)
        : brightness(b), pos(p), color(c), normal(n), uv(uv_coords) {
    }

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, normal);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, uv);

        return attributeDescriptions;
    }
};