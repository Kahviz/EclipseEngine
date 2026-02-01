#include "MakeGui.h"
#include <Windows.h>
#include <algorithm>
#include "imgui.h"
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include "GLOBALS.h"

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
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("Viewport", nullptr,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse
    );

    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    static ImVec2 previousSize = ImGui::GetContentRegionAvail();

#if INEDITOR == 1
    viewport_height = viewportSize.y;
    viewport_width = viewportSize.x;
#endif

    /*
    ID3D11ShaderResourceView* srv = wnd.GetGraphics().GetSceneSRV();

    if (srv) {
        ImVec2 currentSize = ImGui::GetWindowSize();

        if (currentSize.x != previousSize.x || currentSize.y != previousSize.y)
        {
            
        }
        ImGui::Image((ImTextureID)srv, viewportSize, ImVec2(0, 0), ImVec2(1, 1));
       
    }
    else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "No Viewport (No ID3D11ShaderResourceView)  I donno what to do....");
    }
    */
    ImGui::End();
    ImGui::PopStyleVar();

}