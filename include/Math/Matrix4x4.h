#pragma once

#include <iostream>

struct Matrix4x4
{
	Vector4 x;
	Vector4 y;
	Vector4 z;
	Vector4 w;
};

inline std::ostream& operator<<(std::ostream& os, const Matrix4x4& v)
{
	return os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}

inline Matrix4x4 operator+(const Matrix4x4& a, const Matrix4x4& b)
{
	return Matrix4x4{
		a.x + b.x,
		a.y + b.y,
		a.z + b.z,
		a.w + b.w
	};
}

inline Matrix4x4 operator-(const Matrix4x4& a, const Matrix4x4& b)
{
	return Matrix4x4{
		a.x - b.x,
		a.y - b.y,
		a.z - b.z,
		a.w - b.w
	};
}

inline Matrix4x4 operator*(const Matrix4x4& a, float scalar)
{
	return Matrix4x4{
		a.x * scalar,
		a.y * scalar,
		a.z * scalar,
		a.w * scalar
	};
}

inline Matrix4x4 operator/(const Matrix4x4& a, float scalar)
{
	return Matrix4x4{ a.x / scalar, a.y / scalar, a.z / scalar , a.w / scalar };
}

inline Matrix4x4 MatrixRotationRollPitchYaw(float pitch, float yaw, float roll) {
	float cp = cosf(pitch), sp = sinf(pitch);
	float cy = cosf(yaw), sy = sinf(yaw);
	float cr = cosf(roll), sr = sinf(roll);

	Matrix4x4 m;

	m.x = Vector4(cr * cy + sr * sp * sy, sr * cp, -cr * sy + sr * sp * cy, 0);
	m.y = Vector4(-sr * cy + cr * sp * sy, cr * cp, sr * sy + cr * sp * cy, 0);
	m.z = Vector4(cp * sy, -sp, cp * cy, 0);
	m.w = Vector4(0, 0, 0, 1);

	return m;
}

inline Matrix4x4 MatrixPerspectiveFovLH(float fovRadians, float aspect, float nearZ, float farZ) {
	float f = 1.0f / tanf(fovRadians / 2.0f);

	Matrix4x4 m = {};

	m.x = { f / aspect, 0, 0, 0 };
	m.y = { 0, f, 0, 0 };
	m.z = { 0, 0, farZ / (farZ - nearZ), 1 };
	m.w = { 0, 0, -nearZ * farZ / (farZ - nearZ), 0 };

	return m;
}


inline Matrix4x4 IdentityMatrix4x4() {
	return Matrix4x4{
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0}
	};
}

inline float Determinant3x3(float a1, float a2, float a3,
	float b1, float b2, float b3,
	float c1, float c2, float c3)
{
	return a1 * (b2 * c3 - b3 * c2) - a2 * (b1 * c3 - b3 * c1) + a3 * (b1 * c2 - b2 * c1);
}


inline Matrix4x4 InverseMatrix(const Matrix4x4& m) {
	Matrix4x4 inv;
	float det;

	inv.x.x = Determinant3x3(m.y.y, m.y.z, m.y.w, m.z.y, m.z.z, m.z.w, m.w.y, m.w.z, m.w.w);
	inv.x.y = -Determinant3x3(m.y.x, m.y.z, m.y.w, m.z.x, m.z.z, m.z.w, m.w.x, m.w.z, m.w.w);
	inv.x.z = Determinant3x3(m.y.x, m.y.y, m.y.w, m.z.x, m.z.y, m.z.w, m.w.x, m.w.y, m.w.w);
	inv.x.w = -Determinant3x3(m.y.x, m.y.y, m.y.z, m.z.x, m.z.y, m.z.z, m.w.x, m.w.y, m.w.z);

	inv.y.x = -Determinant3x3(m.x.y, m.x.z, m.x.w, m.z.y, m.z.z, m.z.w, m.w.y, m.w.z, m.w.w);
	inv.y.y = Determinant3x3(m.x.x, m.x.z, m.x.w, m.z.x, m.z.z, m.z.w, m.w.x, m.w.z, m.w.w);
	inv.y.z = -Determinant3x3(m.x.x, m.x.y, m.x.w, m.z.x, m.z.y, m.z.w, m.w.x, m.w.y, m.w.w);
	inv.y.w = Determinant3x3(m.x.x, m.x.y, m.x.z, m.z.x, m.z.y, m.z.z, m.w.x, m.w.y, m.w.z);

	inv.z.x = Determinant3x3(m.x.y, m.x.z, m.x.w, m.y.y, m.y.z, m.y.w, m.w.y, m.w.z, m.w.w);
	inv.z.y = -Determinant3x3(m.x.x, m.x.z, m.x.w, m.y.x, m.y.z, m.y.w, m.w.x, m.w.z, m.w.w);
	inv.z.z = Determinant3x3(m.x.x, m.x.y, m.x.w, m.y.x, m.y.y, m.y.w, m.w.x, m.w.y, m.w.w);
	inv.z.w = -Determinant3x3(m.x.x, m.x.y, m.x.z, m.y.x, m.y.y, m.y.z, m.w.x, m.w.y, m.w.z);

	inv.w.x = -Determinant3x3(m.x.y, m.x.z, m.x.w, m.y.y, m.y.z, m.y.w, m.z.y, m.z.z, m.z.w);
	inv.w.y = Determinant3x3(m.x.x, m.x.z, m.x.w, m.y.x, m.y.z, m.y.w, m.z.x, m.z.z, m.z.w);
	inv.w.z = -Determinant3x3(m.x.x, m.x.y, m.x.w, m.y.x, m.y.y, m.y.w, m.z.x, m.z.y, m.z.w);
	inv.w.w = Determinant3x3(m.x.x, m.x.y, m.x.z, m.y.x, m.y.y, m.y.z, m.z.x, m.z.y, m.z.z);

	det = m.x.x * inv.x.x + m.x.y * inv.y.x + m.x.z * inv.z.x + m.x.w * inv.w.x;
	if (fabs(det) < 1e-6f) throw std::runtime_error("Matrix not invertible!");

	float invDet = 1.0f / det;
	inv.x.x *= invDet; inv.x.y *= invDet; inv.x.z *= invDet; inv.x.w *= invDet;
	inv.y.x *= invDet; inv.y.y *= invDet; inv.y.z *= invDet; inv.y.w *= invDet;
	inv.z.x *= invDet; inv.z.y *= invDet; inv.z.z *= invDet; inv.z.w *= invDet;
	inv.w.x *= invDet; inv.w.y *= invDet; inv.w.z *= invDet; inv.w.w *= invDet;

	return inv;
}
