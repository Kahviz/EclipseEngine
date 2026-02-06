
#include <vector>
#include <memory>
#include <functional>
#include "imgui.h"
#include "Window/Window.h"
#include "GLOBALS.h"
#include "MakeGui.h"

class Instance;

class Engine {
public:
    Engine();
    ~Engine();

    int EngineRun();
    Instance& AddAMesh(const std::string& Path, const std::string& Name, FLOAT3 pos, FLOAT3 Size, bool Selec);

private:
    void EngineDoFrame(Window* wnd, float deltatime);
    bool ImGuiInited = false;

    MakeGui makeGui;
    Window window;
    std::vector<std::unique_ptr<Instance>> Drawables;
    int Index = 0;
    int screen_width = 1280;
    int screen_height = 800;
    FLOAT3 Color3 = { 1.0f, 1.0f, 1.0f };
};