#pragma once

#include "Graphics/Graphics.h"
#include "GLFW/glfw3.h"
#include "Camera/Camera.h"
#include "Window/Window.h"

class CameraControl
{
public:
	void MakeCameraControls(Window& wnd, float deltaTime);
	Matrix4x4 GetViewMatrix(const Camera& cam);
	Matrix4x4 GetProjectionMatrix(const Camera& cam, float aspectRatio);
private:
};