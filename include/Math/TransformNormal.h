#pragma once

#include "Vector4.h"
#include "Matrix4x4.h"
#include "Vector3.h"

inline Vector4 TransformNormal(const Vector4& v, const Matrix4x4& m)
{
	Vector4 result;
	result.x = v.x * m.x.x + v.y * m.y.x + v.z * m.z.x;
	result.y = v.x * m.x.y + v.y * m.y.y + v.z * m.z.y;
	result.z = v.x * m.x.z + v.y * m.y.z + v.z * m.z.z;
	result.w = 0.0f;
	return result;
}

inline Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m)
{
	Vector3 result;
	result.x = v.x * m.x.x + v.y * m.y.x + v.z * m.z.x;
	result.y = v.x * m.x.y + v.y * m.y.y + v.z * m.z.y;
	result.z = v.x * m.x.z + v.y * m.y.z + v.z * m.z.z;
	return result;
}