#pragma once

#include <cmath>
#include <limits>

#include "Math/Matrix4x4.h"
#include "Math/Vector4.h"
#include "INT3.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
//Funcs
#include "StoreFloat.h"
#include "MatrixLookAtLH.h";
#include "LoadINT.h"

//Utils
#include "M_Utils.h"
//Variables
#include "Variables.h"

inline Vector4 LoadFloat3(Vector3 a) {
	return Vector4(
		a.x,
		a.y,
		a.z,
		0.0f
	);
}
