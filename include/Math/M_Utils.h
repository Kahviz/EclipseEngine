#pragma once

inline float DegreesToRadians(float degrees)
{
	return degrees * (PI / 180.0f);
}

inline float RadiansToDegrees(float radians)
{
	return radians * (180.0f / PI);
}