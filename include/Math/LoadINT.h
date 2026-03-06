#pragma once 
#include "Vector4.h"
#include "INT3.h"

inline Vector4 LoadInt3(INT3 a) {
	return Vector4(
		static_cast<float>(a.x),
		static_cast<float>(a.y),
		static_cast<float>(a.z),
		0.0f
	);
}