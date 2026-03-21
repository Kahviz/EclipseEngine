#pragma once
#include <array>
#include "Math/UntilitedMath.h"
#include "Vulkan/vulkan.h"

struct Vertex
{
    float brightness;  // offset 0
    Vector3 pos;        // offset 4
    Vector3 color;      // offset 16
    Vector3 normal;     // offset 28
    Vector2 uv;
    Vertex() = default;

    Vertex(float b, const Vector3& p, const Vector3& c, const Vector3& n)
        : brightness(b), pos(p), color(c), normal(n) {
    }

    Vertex(float b, Vector3& p, Vector3&& c, Vector3&& n)
        : brightness(b), pos(std::move(p)), color(std::move(c)), normal(std::move(n)) {
    }

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

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
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, uv);

        return attributeDescriptions;
    }
};