#include "Raycasting.h"
#include "Mesh/Mesh.h"

bool Raycasting::IsClicked(Instance* inst, const Vector3& rayOrigin, const Vector3& rayDir)
{
    return inst->RayIntersects(rayOrigin, rayDir);
}

bool Raycasting::RayIntersectsTriangle(
    const Vector3& rayOrigin,
    const Vector3& rayDir,
    const Vector3& v0,
    const Vector3& v1,
    const Vector3& v2,
    float& tOut
) {
    const float T_MIN = 0.001f;
    const float T_MAX = 1e6f;
    const float PARALLEL_EPSILON = 1e-8f;
    const float BARY_EPSILON = 1e-4f;

    Vector3 edge1 = v1 - v0;
    Vector3 edge2 = v2 - v0;

    Vector3 h = Cross(rayDir, edge2);
    float a = dot(edge1, h);
    if (fabsf(a) <= PARALLEL_EPSILON)
        return false;

    float f = 1.0f / a;
    Vector3 s = rayOrigin - v0;
    float u = f * dot(s, h);
    if (u < -BARY_EPSILON || u > 1.0f + BARY_EPSILON)
        return false;

    Vector3 q = Cross(s, edge1);
    float v = f * dot(rayDir, q);
    if (v < -BARY_EPSILON || (u + v) > 1.0f + BARY_EPSILON)
        return false;

    float t = f * dot(edge2, q);
    if (t <= T_MIN || t >= T_MAX)
        return false;

    // Optional: normal-based check
    Vector3 normal = Normalize(Cross(edge1, edge2));
    float cosAngle = fabsf(dot(normal, rayDir));
    if (cosAngle < 0.001f)
        return false;

    tOut = t;
    return true;
}
