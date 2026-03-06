#pragma once

#include "FLOAT3.h"
#include "Vector4.h"

inline void StoreFloat3(FLOAT3* dest, const Vector4& src) {
	if (!dest) return;
	dest->x = src.x;
	dest->y = src.y;
	dest->z = src.z;
}