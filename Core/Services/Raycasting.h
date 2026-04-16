#pragma once
#include "BoronMathLibrary.h"
#include "Instances/Instance.h"

class Instance;

class Raycasting {
public:
    bool IsClicked(Instance* inst, const Vector3& rayOrigin, const Vector3& rayDir);
    bool RayIntersectsTriangle(const Vector3& rayOrigin, const Vector3& rayDir, const Vector3& v0, const Vector3& v1, const Vector3& v2, float& tOut);
};