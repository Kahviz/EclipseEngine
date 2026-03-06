#include "MakeGui.h"
#include <Windows.h>
#include <algorithm>
#include "imgui.h"
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include "GLOBALS.h"
#include "Graphics/Graphics.h"

struct MeshButton
{
    const char* label;
    std::string path;
    const char* name;
};

static MeshButton meshButtons[] =
{
    { "Cube",     assets + "\\Cube.obj","Cube" },
    { "Ball",     assets + "\\Ball.obj",  "Ball" },
    { "Cylinder", assets + "\\Cylinder.obj", "Cylinder"}
};

void MakeChildrenNodes(Instance* inst) {
    if (!inst) return;

    std::vector<Instance*> children = inst->GetChildren();
    for (auto& child : children) {
        if (!child)
            continue;

        std::vector<Instance*> grandChildren = child->GetChildren();
        if (grandChildren.empty())
        {
            ImGui::TreeNodeEx((void*)child,
                ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
                "%s",
                child->Name.c_str());
        }
        else
        {
            if (ImGui::TreeNodeEx((void*)child, ImGuiTreeNodeFlags_OpenOnArrow, "%s", child->Name.c_str()))
            {
                MakeChildrenNodes(child);
                ImGui::TreePop();
            }
        }
    }
}

FLOAT3 MakeVec3TextEdit(Instance* inst,
    const std::string& name,
    const std::string& VecType)
{
    return { 0,0,0 };
}

void MakeGui::MakeStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    /*
    ImGui::Begin("Color Picker");

    ImGui::ColorEdit3("Clear Color", (float*)&clear_color);

    ImGui::ColorEdit4("Clear Color with Alpha", (float*)&clear_color);

    ImGui::End();
    */

    colors[ImGuiCol_ButtonHovered] = ImVec4(0.065f, 0.065f, 0.065f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.055f, 0.055f, 0.055f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.06f, 0.05f, 1.0f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.0f);
    colors[ImGuiCol_Border] = ImVec4(0.07f, 0.07f, 0.07f, 1.0f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.015f, 0.015f, 0.015f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);

    style.FrameRounding = FrameRounding;
    style.WindowRounding = WindowRounding;
    style.ChildRounding = ChildRounding;
    style.PopupRounding = PopupRounding;
    style.GrabRounding = GrabRounding;
}

void MakeGui::MakeIMGui(Window& wnd, std::vector<std::unique_ptr<Instance>>& Drawables, std::function<Instance* (const std::string&, const std::string&, FLOAT3, FLOAT3, bool)> AddAMesh, float* Color3, bool Selec)
{ 
    MakeStyle();
    
    world.Name = "World";
    world.Parent = nullptr;
    world.CodeTag = "World";

    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = 0;

    static bool Switch = false;
    static bool CanChange = true;

    static ImVec2 LastSize2;
    static ImVec2 LastSize1;

    if (!Switch) {
        LastSize1 = io.DisplaySize;
        Switch = true;

        if (LastSize1.x != LastSize2.x || LastSize1.y != LastSize2.y) {
            CanChange = true;
        }
    }
    else {
        LastSize2 = io.DisplaySize;
        Switch = false;

        if (LastSize1.x != LastSize2.x || LastSize1.y != LastSize2.y) {
            CanChange = true;
        }
    }

    float screen_w = io.DisplaySize.x;
    float screen_h = io.DisplaySize.y;

    float window_w = screen_w / 5.0f;
    float window_h = screen_h / 2.0f;

    if (CanChange) {
        ImGui::SetNextWindowSize(ImVec2(window_w, window_h), ImGuiCond_Always);

        ImGui::SetNextWindowPos(ImVec2(screen_w - window_w / 1.06f, screen_h / 4.0f), ImGuiCond_Always);
    }

    ImGui::Begin("Explorer", nullptr,
        ImGuiWindowFlags_AlwaysHorizontalScrollbar |
        ImGuiWindowFlags_AlwaysVerticalScrollbar
    );

    if (ImGui::TreeNodeEx("World", ImGuiTreeNodeFlags_OpenOnArrow))
    {
        for (auto& instPtr : Drawables)
        {
            Instance* inst = instPtr.get();
            inst->Parent = &world;

            if (!inst || inst->Parent != &world)
                continue;

            if (ImGui::TreeNodeEx(
                (void*)inst,
                ImGuiTreeNodeFlags_OpenOnArrow,
                "%s",
                inst->Name.c_str()))
            {
                MakeChildrenNodes(inst);
                ImGui::TreePop();
            }

            if (ImGui::IsItemClicked())
            {
                for (auto& i : Drawables)
                    i->Deselect();

                inst->Select();
            }
        }
        ImGui::TreePop();
    }

    ImGui::End();

    if (CanChange) {

        ImGui::SetNextWindowSize(ImVec2(screen_w, screen_h / 4.0f), ImGuiCond_Always);

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    }

    if (ImGui::Begin("UntilitedGameEngine", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize)) {
        if (ImGui::BeginTabBar("##TABS")) {
            if (ImGui::BeginTabItem("Home")) {
                if (ImGui::Button("Move",ImVec2(screen_w / 10,screen_h / 10))) {

                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();

        }
    }

    ImGui::End();

    CanChange = false;
}

void MakeGui::MakeIMViewPort(Window& wnd)
{
}

bool MakeGui::MakeDashBoard()
{
    static bool showConfigWindow = false;

    MakeStyle();
    ImGui::SetNextWindowSize(ImVec2(screen_width, screen_height), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f,0.0f,0.0f, 1.0f)); //Color

    float sh = screen_height;
    float sw = screen_width;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    ImGui::Begin("Backround", nullptr,
        ImGuiWindowFlags_NoTitleBar | //No TitleBar
        ImGuiWindowFlags_NoResize |   //No resize
        ImGuiWindowFlags_NoMove |     //No Moving
        ImGuiWindowFlags_NoCollapse |  //No Smalling
        ImGuiWindowFlags_NoDocking
    );

    ImGui::PopStyleColor();

    ImVec2 windowSize = ImGui::GetWindowSize();
    float size = min(windowSize.x, windowSize.y);

    ImVec2 buttonSize = ImVec2(size / 8, size / 8);
    ImVec2 NewProjectSize = ImVec2(sw / 8, sh / 16);

    ImGui::SetCursorPos(ImVec2(
        (windowSize.x + sw / 5 - NewProjectSize.x) * 0.5f,
        (windowSize.y - NewProjectSize.y + sh / 5) * 0.5f
    ));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.086f, 0.769f, 0.212f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.096f, 0.869f, 0.312f,1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.076f, 0.669f, 0.112f,1.0f));

    if (ImGui::Button("New Project", NewProjectSize)) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        ImGui::End();
        return true;
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::SetCursorPos(ImVec2(
        sw / 100,
        sh / 100
    ));

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(sw / 5, sh));
    ImGui::Begin("Discover ##1", nullptr,
        ImGuiWindowFlags_AlwaysVerticalScrollbar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoDocking
    );
    if (ImGui::Button("Config Style")) {
        showConfigWindow = !showConfigWindow;
    }
    ImGui::End();

    if (showConfigWindow) {
        ImGui::SetNextWindowSize(ImVec2(sw / 2, sh), ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); //Color

        ImGui::SetNextWindowPos(ImVec2(sw / 2,0), ImGuiCond_Always);

        ImGui::Begin("Test Area", nullptr,
            ImGuiWindowFlags_NoResize |   //No resize
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoDocking
        );

        ImGui::Button("Test Button", ImVec2(size / 10, size / 10));
        ImGui::PopStyleColor();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(sw / 2, sh));
        ImGui::SetNextWindowPos(ImVec2(0, 0));

        ImGui::Begin("Style --Press Ctrl-Z to Go Back", nullptr,
            ImGuiWindowFlags_AlwaysVerticalScrollbar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse
        );

        bool ctrlPressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
        bool zPressed = ImGui::IsKeyPressed(ImGuiKey_Z);
        if (ctrlPressed && zPressed) {
            showConfigWindow = false;
        }

        if (ImGui::Button("Go Back")) {
            showConfigWindow = false;
        }
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

        ImGui::SliderFloat("FrameRounding##Slider", &FrameRounding, 0.0f, 100.0f, "%.2f");
        ImGui::InputFloat("FrameRounding##Input", &FrameRounding);

        if (ImGui::Button("Reset ##1")) {
            FrameRounding = Default_FrameRounding;
        }

        ImGui::SliderFloat("WindowRounding##Slider", &WindowRounding, 0.0f, 100.0f, "%.2f");
        ImGui::InputFloat("WindowRounding##Input", &WindowRounding);
        
        if (ImGui::Button("Reset ##2")) {
            WindowRounding = Default_WindowRounding;
        }

        ImGui::SliderFloat("ChildRounding##Slider", &ChildRounding, 0.0f, 100.0f, "%.2f");
        ImGui::InputFloat("ChildRounding##Input", &ChildRounding);

        if (ImGui::Button("Reset ##3")) {
            ChildRounding = Default_ChildRounding;
        }

        ImGui::SliderFloat("GrabRounding##Slider", &GrabRounding, 0.0f, 100.0f, "%.2f");
        ImGui::InputFloat("GrabRounding##Input", &GrabRounding);

        if (ImGui::Button("Reset ##4")) {
            GrabRounding = Default_GrabRounding;
        }

        ImGui::PopStyleColor();
        ImGui::End();
    }

    ImGui::End();

    return false;
}