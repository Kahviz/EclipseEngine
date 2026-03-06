#pragma once

#include <cmath>

#include "Matrix4x4.h"
#include "Vector3.h"

inline Matrix4x4 MatrixLookAtLH(const Vector3& eye, const Vector3& target, const Vector3& up) {
	Vector3 zaxis = { target.x - eye.x, target.y - eye.y, target.z - eye.z };
	float len = sqrtf(zaxis.x * zaxis.x + zaxis.y * zaxis.y + zaxis.z * zaxis.z);
	zaxis.x /= len; zaxis.y /= len; zaxis.z /= len;

	Vector3 xaxis = {
		up.y * zaxis.z - up.z * zaxis.y,
		up.z * zaxis.x - up.x * zaxis.z,
		up.x * zaxis.y - up.y * zaxis.x
	};
	len = sqrtf(xaxis.x * xaxis.x + xaxis.y * xaxis.y + xaxis.z * xaxis.z);
	xaxis.x /= len; xaxis.y /= len; xaxis.z /= len;

	Vector3 yaxis = {
		zaxis.y * xaxis.z - zaxis.z * xaxis.y,
		zaxis.z * xaxis.x - zaxis.x * xaxis.z,
		zaxis.x * xaxis.y - zaxis.y * xaxis.x
	};

	Matrix4x4 m;
	m.x = VectorSet(xaxis.x, yaxis.x, zaxis.x, 0);
	m.y = VectorSet(xaxis.y, yaxis.y, zaxis.y, 0);
	m.z = VectorSet(xaxis.z, yaxis.z, zaxis.z, 0);
	m.w = VectorSet(
		-(xaxis.x * eye.x + xaxis.y * eye.y + xaxis.z * eye.z),
		-(yaxis.x * eye.x + yaxis.y * eye.y + yaxis.z * eye.z),
		-(zaxis.x * eye.x + zaxis.y * eye.y + zaxis.z * eye.z),
		1
	);
	return m;
}