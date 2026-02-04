#pragma once
#include <vector>
#include <DirectXMath.h>
#include <string>
#include <memory>
#include "Math/UntilitedMath.h"
#include <Mesh.h>

using namespace DirectX;

class Instance {
public:
    Mesh OBJmesh;
    FLOAT3 Velocity;

    bool Anchored;

    Instance* Parent = nullptr;
    std::vector<Instance*> Children;
    std::string Name;

    FLOAT3 pos = { 0.0f, 0.0f, 0.0f };
    char PosText[64] = "0,0,0";

    //Color
    INT3 color = { 0,0,0 };
    INT3 originalColor = { 0,0,0 };

    FLOAT3 Size = { 1.0f, 1.0f, 1.0f };
    char SizeText[64] = "1,1,1";

    FLOAT3 Orientation = { 0.0f,0.0f,0.0f };
    char OrientationText[64] = "0,0,0";

    int UniqueID = 0;
    bool Selected = false;
    bool Deleted = false;
    std::string CodeTag = "Instance";
    int InstanceID = 0;

    Instance(
        const std::string& name = "",
        const FLOAT3& position = { 0, 0, 0 },
        const FLOAT3& SIZE = { 0, 0, 0 },
        const INT3& COLOR = { 0, 0, 0 },
        const INT3& OGCOLOR = { 0, 0, 0 },
        const FLOAT3& ORIENTATION = { 0, 0, 0 },
        const FLOAT3& VELOCITY = { 0, 0, 0 },
        const Mesh& MESH = Mesh(),
        int uniqueID = 0,
        int instanceID = 0,
        bool selected = false,
        bool deleted = false,
        bool ANCHORED = false,
        const std::string& codeTag = "Instance"
    )
        : Parent(nullptr),
        OBJmesh(MESH),
        Anchored(ANCHORED),
        Name(name),
        Size(SIZE),
        Velocity(VELOCITY),
        color(COLOR),
        originalColor(OGCOLOR),
        pos(position),
        Orientation(ORIENTATION),
        UniqueID(uniqueID),
        InstanceID(instanceID),
        Selected(selected),
        Deleted(deleted),
        CodeTag(codeTag)
    {
    }

    virtual bool CanDraw() const { return false; }
    virtual bool HaveColor() const { return false; }
    virtual bool HaveSize() const { return false; }
    virtual bool HavePos() const { return false; }
    virtual bool HaveOrientation() const { return false; }

    virtual bool HaveVelocity() const { return false; }
    virtual bool HaveAnchored() const { return false; }
    virtual bool HaveOBJMesh() const { return false; }

    virtual bool RayIntersects(const Vector3& rayOrigin, const Vector3& rayDir) { return false; };

    virtual std::vector<Instance*> GetChildren() {
        std::vector<Instance*> ChildrenTable;

        for (auto& Inst : Children) {
            ChildrenTable.push_back(Inst);
        }

        return ChildrenTable;
    }

    virtual void Select() {
        Selected = true;
    }

    virtual bool IsChild() const {
        return Parent != nullptr && Parent->CodeTag != "World";
    }

    virtual void Deselect() {
        Selected = false;
    }
};