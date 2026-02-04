#pragma once

#include "Raycasting.h"

#include <vector>
#include <DirectXMath.h>
#include <string>
#include "Mesh.h"
#include "Instance.h"
#include "Math/UntilitedMath.h"

using namespace DirectX;

inline Vector3 LoadWorldVertex(const Vertex& v, const FLOAT3& objPos, const FLOAT3& objSize) {
    return Vector3(
        v.pos.x * objSize.x + objPos.x,
        v.pos.y * objSize.y + objPos.y,
        v.pos.z * objSize.z + objPos.z
    );
}

class Raycasting;

class Object : public Instance {
public:

    std::string Path;
    std::string CodeTag;

    Object(
        const std::string& name = "",
        int instanceID = 1,
        const FLOAT3& position = { 0.0f,0.0f,0.0f },  // ← NIMI: position
        const FLOAT3& Size = { 1.0f,1.0f,1.0f },
        const INT3& col = { 168, 160, 160 },
        const INT3& ogcolor = { 168, 160, 160 },
        const FLOAT3& VELOCITY = { 0.0f,0.0f,0.0f },
        const FLOAT3& ORIENTATION = { 0.0f,0.0f,0.0f },
        const bool Anchored = false,
        Mesh OBJmesh = Mesh()
    )
        : Instance(name,    // 1. name
            position,       // 2. position
            Size,           // 3. SIZE
            col,            // 4. COLOR
            ogcolor,        // 5. OGCOLOR
            ORIENTATION,    // 6. ORIENTATION
            VELOCITY,       // 7. VELOCITY
            OBJmesh,        // 8. MESH
            UniqueID,              // 9. uniqueID
            instanceID,     // 10. instanceID
            false,          // 11. selected ← KORJATTU: Selected → false
            false,          // 12. deleted ← KORJATTU: Deleted → false
            Anchored,       // 13. ANCHORED
            "Decent"     // 14. codeTag ← KORJATTU: CodeTag → "Decent"
        ), 
        Path("")
    {
        
    }

    bool CanDraw() const override { return true; }
    bool HaveColor() const override { return true; }
    bool HaveSize() const override { return true; }
    bool HavePos() const override { return true; }
    bool HaveOrientation() const override { return true; }

    bool HaveVelocity() const override { return true; }
    bool HaveAnchored() const override { return true; }
    bool HaveOBJMesh() const override { return true; }

    bool RayIntersects(const Vector3& rayOrigin, const Vector3& rayDir) override {
        auto& Vertices = OBJmesh.GetVertices();
        auto& Indices = OBJmesh.GetIndices();

        const FLOAT3& objPos = this->pos;
        const FLOAT3& objSize = this->Size;

        for (size_t i = 0; i < Indices.size(); i += 3) {
            Vector3 v0 = LoadWorldVertex(Vertices[Indices[i]], objPos, objSize);
            Vector3 v1 = LoadWorldVertex(Vertices[Indices[i + 1]], objPos, objSize);
            Vector3 v2 = LoadWorldVertex(Vertices[Indices[i + 2]], objPos, objSize);

            float t;
            Raycasting ray;
            if (ray.RayIntersectsTriangle(rayOrigin, rayDir, v0, v1, v2, t))
                return true;
        }

        
        return false;
    }


    void Deselect() override {
        Selected = false;
        color = originalColor;
    }
};