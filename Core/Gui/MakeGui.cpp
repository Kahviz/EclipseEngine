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

void MakeGui::MakeIMGui(Window& wnd, std::vector<std::unique_ptr<Instance>>& Drawables, std::function<Instance* (const std::string&, const std::string&, FLOAT3, FLOAT3, bool)> AddAMesh, float* Color3, bool Selec)
{ 
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui::Begin("Color Picker");

    ImGui::ColorEdit3("Clear Color", (float*)&clear_color);

    ImGui::ColorEdit4("Clear Color with Alpha", (float*)&clear_color);

    ImGui::End();

    colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.04f,1.0f);
    colors[ImGuiCol_Border] = ImVec4(0.07f, 0.07f, 0.07f,1.0f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.015f, 0.015f, 0.015f, 1.0f);

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
                ImGui::Text("Select, Move,Scale,Rotation,Color,Anhored,Add");
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
    ImGui::SetNextWindowSize(ImVec2(screen_width, screen_height), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f,0.0f,0.0f, 1.0f)); //Color

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    ImGui::Begin("Backround", nullptr,
        ImGuiWindowFlags_NoTitleBar | //No TitleBar
        ImGuiWindowFlags_NoResize |   //No resize
        ImGuiWindowFlags_NoMove |     //No Moving
        ImGuiWindowFlags_NoCollapse   //No Smalling
    );
    ImGui::PopStyleColor();

    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 buttonSize = ImVec2(150, 30);

    if (ImGui::Button("Create A New Project", buttonSize)) {
        ImGui::End();
        return true;
    }

    ImGui::End();

    return false;
}
