#pragma once

#include <iostream>


struct Vector4
{
	float x;
	float y;
	float z;
	float w;
};

inline std::ostream& operator<<(std::ostream& os, const Vector4& v)
{
	return os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}

inline Vector4 operator+(const Vector4& a, const Vector4& b)
{
	return Vector4{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

inline Vector4 operator-(const Vector4& a, const Vector4& b)
{
	return Vector4{ a.x - b.x, a.y - b.y, a.z - b.z,a.w - b.w };
}

inline Vector4 operator*(const Vector4& a, float scalar)
{
	return Vector4{ a.x * scalar, a.y * scalar, a.z * scalar, a.w * scalar };
}

inline Vector4 operator/(const Vector4& a, float scalar)
{
	return Vector4{ a.x / scalar, a.y / scalar, a.z / scalar, a.w / scalar };
}

inline float dot(const Vector4& a, const Vector4& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}


inline Vector4 VectorSet(float x, float y, float z, float w) {
	return Vector4(x, y, z, w);
}

inline Vector4 Normalize(const Vector4& v) {
	float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	if (len == 0.0f) return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	return Vector4(v.x / len, v.y / len, v.z / len, 0.0f);
}