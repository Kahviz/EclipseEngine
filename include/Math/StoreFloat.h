#pragma once

#include "Vector4.h"

inline void StoreFloat3(Vector3* dest, const Vector4& src) {
	if (!dest) return;
	dest->x = src.x;
	dest->y = src.y;
	dest->z = src.z;
}