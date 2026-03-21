#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include "GLOBALS.h"

#include "DX11/MeshDX11.h"
#include "Vulkan/MeshVulkan.h"
#include <Math/UntilitedMath.h>
#include <Instances/Vertex.h>

class Mesh
{
public:
#if DIRECTX11 == 1
    void Load(const std::string& file, ID3D11Device* device) {
        DM.Load(file, device);
    }
    void DrawForDX11(ID3D11DeviceContext* ctx) const {
        DM.Draw(ctx);
    };
#endif
    
#if VULKAN == 1
    void Load(const std::string& file, VkDevice device, VkPhysicalDevice phyDevice, VkCommandPool cmdPool, VkQueue gfxQueue) {
        VM.Load(file, device, phyDevice, cmdPool, gfxQueue);
    }

    void DrawForVulkan(VkCommandBuffer cb)
    {
        VM.Draw(cb);
    }

#endif

    
    /*
    const std::string& file,
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandPool commandPool,
        VkQueue graphicsQueue
    */
    


    const std::vector<uint32_t>& GetIndices() const {
        #if VULKAN == 1
            return VM.GetIndices();
        #else
            return DM.GetIndices();
        #endif
    }

    const std::vector<Vertex>& GetVertices() const {
        #if VULKAN == 1
            return VM.GetVertices();
        #else
            return DM.GetVertices();
        #endif
    }
    #if VULKAN == 1
        MeshVK VM;
    #endif

    #if DIRECTX11 == 1
        MeshDX11 DM;
    #endif
};
