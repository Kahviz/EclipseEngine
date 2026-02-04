#pragma once
#include <d3d11.h>
#include <vector>
#include "Instances/Vertex.h"

class MeshDX11 {
public:
    void Load(const std::string& file, ID3D11Device* device);
    void Draw(ID3D11DeviceContext* ctx) const;

    const std::vector<uint32_t>& GetIndices() const {
        return indices;
    }

    const std::vector<Vertex>& GetVertices() const {
        return verts;
    }
private:
    ID3D11Buffer* vb = nullptr;
    ID3D11Buffer* ib = nullptr;
    UINT indexCount = 0;
    std::vector<Vertex> verts;
    std::vector<uint32_t> indices;
};