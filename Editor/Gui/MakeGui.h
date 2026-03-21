#pragma once
#include <vector>
#include <functional>
#include "Object.h"
#include <DirectXMath.h>
#include "GLOBALS.h"
#include <memory>
#include "Instances/Instance.h"
#include "GLFW/glfw3.h"
#include "Window/Window.h"

class MakeGui
{
public:
    void MakeStyle();
    void MakeIMGui(
        Window& wnd,
        std::vector<std::unique_ptr<Instance>>& Drawables,
        std::function<Instance* (const std::string&,
            const std::string&,
            Vector3, Vector3,
            bool)> AddAMesh,
        float* Color3,
        bool Selec
    );
    void MakeIMViewPort(Window& wnd);
    bool MakeDashBoard();
private:
    Instance world;

    //Rounding
    float FrameRounding = 5.0f;
    float Default_FrameRounding = 5.0f;

    float WindowRounding = 5.0f;
    float Default_WindowRounding = 5.0f;

    float ChildRounding = 5.0f;
    float Default_ChildRounding = 5.0f;

    float PopupRounding = 5.0f;
    float Default_PopupRounding = 5.0f;

    float GrabRounding = 3.0f;
    float Default_GrabRounding = 3.0f;

};
