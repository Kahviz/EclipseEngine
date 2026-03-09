#pragma once
#include <vector>
#include <DirectXMath.h>
#include <string>
#include "Mesh.h"
#include "Instances/Instance.h"

using namespace DirectX;

class World : public Instance {
public:
    std::string CodeTag;

    World(
        const std::string& name = "",
        int instanceID = 1,
        const XMFLOAT3& position = { 0.0f,0.0f,0.0f },
        const XMFLOAT3& size = { 1.0f,1.0f,1.0f },
        const XMINT3& col = { 0,0,0 }
    )
        : CodeTag("Decent")
    {
        Selected = false;
        Deleted = false;
        UniqueID = 0;
    }

    bool CanDraw() const override { return false; }

    void Deselect() override {
        Selected = false;
    }
};