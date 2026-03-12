#include "MeshVulkan.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include "Vulkan/VulkanHelpers.h"
#include "Instances/Instance.h"
#include "ErrorHandling/ErrorMessage.h"

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
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_OptimizeMeshes
    );

    if (!scene || !scene->HasMeshes())
        throw std::runtime_error("Failed to load model: " + std::string(imp.GetErrorString()));


    aiMesh* m = scene->mMeshes[0];
    if (!m || m->mNumVertices == 0)
        throw std::runtime_error("Invalid mesh data");

    if (!m->HasNormals())
        throw std::runtime_error("Mesh has no normals");

    verts.resize(m->mNumVertices);
    for (uint32_t i = 0; i < m->mNumVertices; ++i)
    {
        verts[i].brightness = 1.0f;

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

        verts[i].color = { 1, 1, 1 };
        if (m->HasTextureCoords(0))
        {
            verts[i].uv = {
                m->mTextureCoords[0][i].x,
                m->mTextureCoords[0][i].y
            };
        }
        else
        {
            MakeAError("No uv");
            verts[i].uv = { 0.0f, 0.0f };
        }
    }

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

    //Staging buffers
    VkBuffer vStaging, iStaging;
    VkDeviceMemory vStagingMem, iStagingMem;

    CreateBuffer(
        vSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vStaging,
        vStagingMem,
        device, physicalDevice
    );

    CreateBuffer(
        iSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        iStaging,
        iStagingMem,
        device,
        physicalDevice
    );

    void* data;
    vkMapMemory(device, vStagingMem, 0, vSize, 0, &data);
    memcpy(data, verts.data(), (size_t)vSize);
    vkUnmapMemory(device, vStagingMem);

    vkMapMemory(device, iStagingMem, 0, iSize, 0, &data);
    memcpy(data, indices.data(), (size_t)iSize);
    vkUnmapMemory(device, iStagingMem);

    CreateBuffer(
        vSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer,
        vertexMemory,
        device,
        physicalDevice
    );

    CreateBuffer(
        iSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indexBuffer,
        indexMemory,
        device,
        physicalDevice
    );

    CopyBuffer(
        vStaging,        // src
        vertexBuffer,    // dst
        vSize,           // size
        commandPool,     // cmdp
        device,          // device
        graphicsQueue    // gQ
    );
    CopyBuffer(
        iStaging,
        indexBuffer,
        iSize,
        commandPool,
        device,
        graphicsQueue
    );


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