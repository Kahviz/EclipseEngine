#include "CameraControl.h"
#include <string>
#include "GLOBALS.h"

#include "CameraControl.h"
#include <GLFW/glfw3.h>
#include "GLOBALS.h"

void CameraControl::MakeCameraControls(Window& wnd, float deltaTime)
{
    float speed = 5.0f * deltaTime;
    float sensitivity = 0.002f;

    Camera& Cam = wnd.GetGraphics().GetCamera();
    FLOAT3 forward = Cam.GetForward();
    FLOAT3 right = Cam.GetRight();

    GLFWwindow* glfwWND = wnd.GetWindow();

    if (glfwGetKey(glfwWND, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        speed = 100.0f * deltaTime;
    }

    if (glfwGetKey(glfwWND, GLFW_KEY_A) == GLFW_PRESS)
        Cam.AdjustPosition(-right.x * speed, -right.y * speed, -right.z * speed);

    if (glfwGetKey(glfwWND, GLFW_KEY_D) == GLFW_PRESS)
        Cam.AdjustPosition(right.x * speed, right.y * speed, right.z * speed);

    if (glfwGetKey(glfwWND, GLFW_KEY_S) == GLFW_PRESS)
        Cam.AdjustPosition(-forward.x * speed, -forward.y * speed, -forward.z * speed);

    if (glfwGetKey(glfwWND, GLFW_KEY_W) == GLFW_PRESS)
        Cam.AdjustPosition(forward.x * speed, forward.y * speed, forward.z * speed);

    if (glfwGetKey(glfwWND, GLFW_KEY_Q) == GLFW_PRESS)
        Cam.AdjustPosition(0.0f, -speed, 0.0f);

    if (glfwGetKey(glfwWND, GLFW_KEY_E) == GLFW_PRESS)
        Cam.AdjustPosition(0.0f, speed, 0.0f);

    static double mouseX, mouseY;
    static double lastX, lastY;
    static bool firstmouse = true;


    glfwGetCursorPos(glfwWND, &mouseX, &mouseY);

    if (firstmouse)
    {
        lastX = mouseX;
        lastY = mouseY;
        firstmouse = false;
    }

    float deltaX = mouseX - lastX;
    float deltaY = mouseY - lastY;

    lastX = mouseX;
    lastY = mouseY;

    if (glfwGetMouseButton(glfwWND, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        Cam.AdjustRotation(deltaY * sensitivity, deltaX * sensitivity, 0.0f);
    }

}

Matrix4x4 CameraControl::GetViewMatrix(const Camera& cam)
{
    return cam.GetViewMatrix();
}

Matrix4x4 CameraControl::GetProjectionMatrix(const Camera& cam, float aspectRatio)
{
    return cam.GetProjectionMatrix();
}