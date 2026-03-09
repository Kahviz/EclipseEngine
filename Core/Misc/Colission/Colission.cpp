#include "Colission.h"
#include <vector>
#include <cmath>
#include "GLOBALS.h"
#include "Instances/Vertex.h"
#include "Instances/Instance.h"

namespace {

    bool SegmentIntersectsTriangle(const Vec3& p0, const Vec3& p1, const Vec3& v0, const Vec3& v1, const Vec3& v2) {
        const float EPSILON = 1e-6f;
        Vec3 dir = sub(p1, p0);
        Vec3 edge1 = sub(v1, v0);
        Vec3 edge2 = sub(v2, v0);
        Vec3 h = cross(dir, edge2);
        float a = dot(edge1, h);
        if (std::fabs(a) < EPSILON) return false;
        float f = 1.0f / a;
        Vec3 s = sub(p0, v0);
        float u = f * dot(s, h);
        if (u < 0.0f || u > 1.0f) return false;
        Vec3 q = cross(s, edge1);
        float v = f * dot(dir, q);
        if (v < 0.0f || (u + v) > 1.0f) return false;
        float t = f * dot(edge2, q);

        if (t < 0.0f || t > 1.0f) return false;
        return true;
    }

    bool PointInTriangle3D(const Vec3& pt, const Vec3& v0, const Vec3& v1, const Vec3& v2) {
        const float EPSILON = 1e-6f;
        Vec3 edge1 = sub(v1, v0);
        Vec3 edge2 = sub(v2, v0);
        Vec3 n = cross(edge1, edge2);
        float nlen2 = dot(n, n);
        if (nlen2 < EPSILON) return false;

        float dist = dot(n, sub(pt, v0));
        if (std::fabs(dist) > 1e-4f * std::sqrt(nlen2)) return false;

        // Barycentric coordinates
        Vec3 v0p = sub(pt, v0);
        float dot00 = dot(edge2, edge2);
        float dot01 = dot(edge2, edge1);
        float dot02 = dot(edge2, v0p);
        float dot11 = dot(edge1, edge1);
        float dot12 = dot(edge1, v0p);

        float denom = dot00 * dot11 - dot01 * dot01;
        if (std::fabs(denom) < EPSILON) return false;
        float invDenom = 1.0f / denom;
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

        return (u >= -EPSILON) && (v >= -EPSILON) && (u + v <= 1.0f + EPSILON);
    }

    Vec3 TransformVertex(const Vertex& Vertex, const Instance& inst)
    {
        Vec3 v = makeVec3FromVertex(Vertex);
        Vec3 scaled = mul(v, mul(Vec3{
            inst.Size.x,
            inst.Size.y,
            inst.Size.z
            }, 1.001f));

        return add(
            Vec3{ inst.pos.x, inst.pos.y, inst.pos.z },
            scaled
        );
    }

    inline Vec3 TriangleNormal(const Vec3& v0, const Vec3& v1, const Vec3& v2)
    {
        Vec3 e1 = sub(v1, v0);
        Vec3 e2 = sub(v2, v0);
        Vec3 n = cross(e1, e2);

        float len = std::sqrt(dot(n, n));
        if (len > 1e-6f)
            return mul(n, 1.0f / len);

        return Vec3{ 0, 0, 0 };
    }


    
}

bool Colission::AABBOverlap(const Instance& a, const Instance& b) {
    float aMinX = a.pos.x;
    float aMaxX = a.pos.x + a.Size.x;
    float aMinY = a.pos.y;
    float aMaxY = a.pos.y + a.Size.y;
    float aMinZ = a.pos.z;
    float aMaxZ = a.pos.z + a.Size.z;

    float bMinX = b.pos.x;
    float bMaxX = b.pos.x + b.Size.x;
    float bMinY = b.pos.y;
    float bMaxY = b.pos.y + b.Size.y;
    float bMinZ = b.pos.z;
    float bMaxZ = b.pos.z + b.Size.z;

    return (aMinX <= bMaxX && aMaxX >= bMinX) &&
        (aMinY <= bMaxY && aMaxY >= bMinY) &&
        (aMinZ <= bMaxZ && aMaxZ >= bMinZ);
}

bool Colission::CheckColission(
    const Instance& inst,
    const Instance& inst2,
    Vec3& outNormal
)
{
    if (!AABBOverlap(inst, inst2))
        return false;

    const auto& verts1 = inst.OBJmesh.GetVertices();
    const auto& verts2 = inst2.OBJmesh.GetVertices();

    if (verts1.empty() || verts2.empty())
        return false;

    size_t triCount1 = verts1.size() / 3;
    size_t triCount2 = verts2.size() / 3;

    for (size_t i = 0; i < triCount1; ++i)
    {
        Vec3 a0 = TransformVertex(verts1[i * 3 + 0], inst);
        Vec3 a1 = TransformVertex(verts1[i * 3 + 1], inst);
        Vec3 a2 = TransformVertex(verts1[i * 3 + 2], inst);

        Vec3 normalA = TriangleNormal(a0, a1, a2);

        for (size_t j = 0; j < triCount2; ++j)
        {
            Vec3 b0 = TransformVertex(verts2[j * 3 + 0], inst2);
            Vec3 b1 = TransformVertex(verts2[j * 3 + 1], inst2);
            Vec3 b2 = TransformVertex(verts2[j * 3 + 2], inst2);

            Vec3 normalB = TriangleNormal(b0, b1, b2);

            if (SegmentIntersectsTriangle(a0, a1, b0, b1, b2) ||
                SegmentIntersectsTriangle(a1, a2, b0, b1, b2) ||
                SegmentIntersectsTriangle(a2, a0, b0, b1, b2))
            {
                outNormal = normalB;
                return true;
            }

            // B:n reunat A:ta vasten
            if (SegmentIntersectsTriangle(b0, b1, a0, a1, a2) ||
                SegmentIntersectsTriangle(b1, b2, a0, a1, a2) ||
                SegmentIntersectsTriangle(b2, b0, a0, a1, a2))
            {
                outNormal = normalA;
                return true;
            }

            if (PointInTriangle3D(a0, b0, b1, b2))
            {
                outNormal = normalB;
                return true;
            }

            if (PointInTriangle3D(b0, a0, a1, a2))
            {
                outNormal = normalA;
                return true;
            }
        }
    }

    return false;
}
