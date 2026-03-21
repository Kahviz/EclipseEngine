#pragma once

#include <vector>
#include <memory>
#include <functional>
#include "imgui.h"
#include "Window/Window.h"
#include "GLOBALS.h"
#if INEDITOR == 1
    #include "Gui/MakeGui.h"
#endif
#include "Debugging/Profiler/Profiler.h"
#include "Instances/Instance.h"
#include "UntilitedPhysics/Physics.h"

class Instance;

class Engine {
public:
    Engine();
    ~Engine();

    int EngineRun();
    Instance& AddAMesh(const std::string& Path, const std::string& Name, Vector3 pos, Vector3 Size, bool Selec);
    
private:
    void EngineDoFrame(Window* wnd, float deltatime);
    bool ImGuiInited = false;

    Physics physics;
    Profiler profiler;
    MakeGui makeGui;
    Window window;
    std::vector<std::unique_ptr<Instance>> Drawables = {};

    bool InProject = false;
    int Index = 0;
    int screen_width = 1280;
    int screen_height = 800;
    Vector3 Color3 = { 1.0f, 1.0f, 1.0f };
};