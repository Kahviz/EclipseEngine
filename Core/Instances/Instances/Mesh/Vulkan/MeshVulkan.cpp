#include "MeshVulkan.h"

#include "MeshVK.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>

void MeshVK::Load(
    const std::string& file,
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkCommandPool commandPool,
    VkQueue graphicsQueue
)
{
    Assimp::Importer imp;
    const aiScene* scene = imp.ReadFile(
        file,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenNormals
    );

    if (!scene || !scene->HasMeshes())
        throw std::runtime_error("Failed to load model: " + std::string(imp.GetErrorString()));

    aiMesh* m = scene->mMeshes[0];
    if (!m || m->mNumVertices == 0)
        throw std::runtime_error("Invalid mesh data");

    if (!m->HasNormals())
        throw std::runtime_error("Mesh has no normals");

    // -------- Vertices --------
    verts.resize(m->mNumVertices);
    for (uint32_t i = 0; i < m->mNumVertices; ++i)
    {
        verts[i].pos = {
            m->mVertices[i].x,
            m->mVertices[i].y,
            m->mVertices[i].z
        };

        verts[i].normal = {
            m->mNormals[i].x,
            m->mNormals[i].y,
            m->mNormals[i].z
        };

        verts[i].color = { 1, 1, 1, 1 };
        verts[i].brightness = 1.0f;
    }

    // -------- Indices --------
    indices.reserve(m->mNumFaces * 3);
    for (uint32_t i = 0; i < m->mNumFaces; ++i)
    {
        const aiFace& face = m->mFaces[i];
        if (face.mNumIndices != 3)
            continue;

        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    indexCount = static_cast<uint32_t>(indices.size());
    if (indexCount == 0)
        throw std::runtime_error("Mesh has no indices");

    VkDeviceSize vSize = sizeof(Vertex) * verts.size();
    VkDeviceSize iSize = sizeof(uint32_t) * indices.size();

    // -------- Staging buffers --------
    VkBuffer vStaging, iStaging;
    VkDeviceMemory vStagingMem, iStagingMem;

    createBuffer(
        physicalDevice, device,
        vSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vStaging, vStagingMem
    );

    createBuffer(
        physicalDevice, device,
        iSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        iStaging, iStagingMem
    );

    void* data;
    vkMapMemory(device, vStagingMem, 0, vSize, 0, &data);
    memcpy(data, verts.data(), (size_t)vSize);
    vkUnmapMemory(device, vStagingMem);

    vkMapMemory(device, iStagingMem, 0, iSize, 0, &data);
    memcpy(data, indices.data(), (size_t)iSize);
    vkUnmapMemory(device, iStagingMem);

    // -------- GPU buffers --------
    createBuffer(
        physicalDevice, device,
        vSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer, vertexMemory
    );

    createBuffer(
        physicalDevice, device,
        iSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indexBuffer, indexMemory
    );

    copyBuffer(device, commandPool, graphicsQueue, vStaging, vertexBuffer, vSize);
    copyBuffer(device, commandPool, graphicsQueue, iStaging, indexBuffer, iSize);

    vkDestroyBuffer(device, vStaging, nullptr);
    vkFreeMemory(device, vStagingMem, nullptr);

    vkDestroyBuffer(device, iStaging, nullptr);
    vkFreeMemory(device, iStagingMem, nullptr);
}

void MeshVK::Draw(VkCommandBuffer cmd) const
{
    VkBuffer vbs[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(cmd, 0, 1, vbs, offsets);
    vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
}
