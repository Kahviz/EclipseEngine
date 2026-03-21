#pragma once
#include <iostream>

struct Vector2
{
	float x;
	float y;
	float z;

	inline Vector2& operator+=(const Vector2& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	inline Vector2& operator-=(const Vector2& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
};

inline std::ostream& operator<<(std::ostream& os, const Vector2& v)
{
	return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

inline Vector2 operator+(const Vector2& a, const Vector2& b)
{
	return Vector2{ a.x + b.x, a.y + b.y, a.z + b.z };
}

inline Vector2 operator-(const Vector2& a, const Vector2& b)
{
	return Vector2{ a.x - b.x, a.y - b.y, a.z - b.z };
}

inline Vector2 operator*(const Vector2& a, float scalar)
{
	return Vector2{ a.x * scalar, a.y * scalar, a.z * scalar };
}

inline Vector2 operator/(const Vector2& a, float scalar)
{
	return Vector2{ a.x / scalar, a.y / scalar, a.z / scalar };
}

inline float dot(const Vector2& a, const Vector2& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float MagnitudeVector2(Vector2 a, Vector2 b) {
	float dx = abs(a.x - b.x);
	float dz = abs(a.z - b.z);
	float dy = abs(a.y - b.y);

	return dx + dz + dy;
}
