#include "MeshDX11.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void MeshDX11::Load(const std::string& file, ID3D11Device* device)
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

    // --- Vertices ---
    verts.resize(m->mNumVertices);
    for (UINT i = 0; i < m->mNumVertices; ++i)
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

    // --- Indices (32-bit) ---
    indices.clear();
    indices.reserve(m->mNumFaces * 3);

    for (UINT i = 0; i < m->mNumFaces; ++i)
    {
        const aiFace& face = m->mFaces[i];
        if (face.mNumIndices != 3)
            continue;

        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    indexCount = static_cast<UINT>(indices.size());

    if (indexCount == 0)
        throw std::runtime_error("Mesh has no indices");

    // --- Vertex buffer ---
    D3D11_BUFFER_DESC vbd{};
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.ByteWidth = sizeof(Vertex) * static_cast<UINT>(verts.size());
    vbd.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA vsd{};
    vsd.pSysMem = verts.data();

    if (FAILED(device->CreateBuffer(&vbd, &vsd, &vb)))
        throw std::runtime_error("Failed to create Vertex buffer");

    // --- Index buffer ---
    D3D11_BUFFER_DESC ibd{};
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.ByteWidth = sizeof(uint32_t) * indexCount;
    ibd.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA isd{};
    isd.pSysMem = indices.data();

    if (FAILED(device->CreateBuffer(&ibd, &isd, &ib)))
        throw std::runtime_error("Failed to create index buffer");
}

void MeshDX11::Draw(ID3D11DeviceContext* ctx) const
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    ctx->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->DrawIndexed(indexCount, 0, 0);
}
