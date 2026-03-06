#pragma once
#include <iostream>

#include "Vector4.h"

struct INT3
{
	int x;
	int y;
	int z;
};

inline std::ostream& operator<<(std::ostream& os, const INT3& v)
{
	return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

inline INT3 operator+(const INT3& a, const INT3& b) {
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

inline INT3 operator-(const INT3& a, const INT3& b)
{
	return INT3{ a.x - b.x, a.y - b.y, a.z - b.z };
}

inline INT3 operator*(const INT3& a, int scalar)
{
	return INT3{ a.x * scalar, a.y * scalar, a.z * scalar };
}

inline INT3 operator/(const INT3& a, int scalar)
{
	return INT3{ a.x / scalar, a.y / scalar, a.z / scalar };
}

inline int dot(const INT3& a, const INT3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline int MagnitudeINT3(INT3 a, INT3 b) {
	int dx = abs(a.x - b.x);
	int dz = abs(a.z - b.z);
	int dy = abs(a.y - b.y);

	return dx + dz + dy;
}

inline void StoreINT3(INT3& Int, Vector4& vec) {
	Int.x = static_cast<int>(vec.x);
	Int.y = static_cast<int>(vec.y);
	Int.z = static_cast<int>(vec.z);
}