#pragma once
#include <vector>
#include <string>
#include <memory>
#include "BoronMathLibrary.h"
#include "Instances/Instances/Mesh/Mesh.h"

class Texture;

class Instance {
public:
    Mesh OBJmesh;
    Vector3 Velocity;

    bool Anchored = true;

    Instance* Parent = nullptr;
    std::vector<Instance*> Children;

    std::string Name;
    char NameText[64] = "Instance";

    Vector3 pos = { 0.0f, 0.0f, 0.0f };
    char PosText[128] = "0,0,0";

    //Color
    Int3 color = { 0,0,0 };
    Int3 originalColor = { 0,0,0 };

    Vector3 Size = { 1.0f, 1.0f, 1.0f };
    char SizeText[128] = "1,1,1";

    Vector3 Orientation = { 0.0f,0.0f,0.0f };
    char OrientationText[128] = "0,0,0";

    int UniqueID = 0;
    bool Selected = false;
    bool Deleted = false;
    std::string CodeTag = "Instance";
    int InstanceID = 0;

    bool IsVisibleInExplorer = false;

    Instance(
        const std::string& name = "Instance",
        const Vector3& position = { 0, 0, 0 },
        const Vector3& SIZE = { 0, 0, 0 },
        const Int3& COLOR = { 0, 0, 0 },
        const Int3& OGCOLOR = { 0, 0, 0 },
        const Vector3& ORIENTATION = { 0, 0, 0 },
        const Vector3& VELOCITY = { 0, 0, 0 },
        const Mesh& MESH = Mesh(),
        int uniqueID = 0,
        int instanceID = 0,
        bool selected = false,
        bool deleted = false,
        bool isvisibleinexplorer = false,
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
        IsVisibleInExplorer(isvisibleinexplorer),
        CodeTag(codeTag)
    {
    }

    virtual Texture* GetTexture() { return nullptr; }
    virtual const Texture* GetConstTexture() const { return nullptr; }
    virtual bool CanDraw() const { return false; }
    virtual bool HasTexture() const { return false; }
    virtual bool HaveColor() const { return false; }
    virtual bool HaveSize() const { return false; }
    virtual bool HavePos() const { return false; }
    virtual bool HaveOrientation() const { return false; }
    virtual bool ShowsInExplorer() const { return false; }
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