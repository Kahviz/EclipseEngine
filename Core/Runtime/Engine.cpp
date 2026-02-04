#include "Engine.h"
#include <stdexcept>
#include "chrono"
#include <iostream>
#include <Instance.h>
#include <Object.h>
#include <GLOBALS.h>
#include <CameraControl.h>
#include "MakeGui.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_dx11.h"
#include <imgui_impl_vulkan.h>


Engine::Engine()
    : window(1280, 800, "UntilitedGameEngine")
{
    if (!ImGuiInited) {
        IMGUI_CHECKVERSION();
        ImGuiContext* context = ImGui::CreateContext();

        ImGui::SetCurrentContext(context);

        ImGui::StyleColorsDark();

        GLFWwindow* glfwWindow = window.GetWindow();
#if DIRECTX11 == 1
    ID3D11Device* device = window.GetGraphics().GetDevice();
    ID3D11DeviceContext* contextDX11 = window.GetGraphics().GetpContext();
#endif

        if (!ImGui_ImplGlfw_InitForOther(glfwWindow, true)) {
            throw std::runtime_error("Failed to initialize ImGui GLFW backend");
        }

        #if DIRECTX11 == 1 
            if (!ImGui_ImplDX11_Init(device, contextDX11)) {
                throw std::runtime_error("Failed to initialize ImGui DX11 backend");
            }
            else {
                ImGuiInited = true;
            }
        #else
            /*
                * if (!ImGui_ImplVulkan_Init()) {
                    throw std::runtime_error("Failed to initialize ImGui Vulkan backend");
                }
                else {
                    ImGuiInited = true;
                }
            */
        #endif

        std::cout << "ImGui initialized successfully!" << std::endl;

        window.SetWindowIcon(window.GetWindow());
        ImGuiIO& IO = ImGui::GetIO();
        IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }
}

Engine::~Engine()
{
    if (ImGuiInited) {
        #if DIRECTX11 == 1
                ImGui_ImplDX11_Shutdown();
        #endif

        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        ImGuiInited = false;
    }
}

int Engine::EngineRun()
{
    GLFWwindow* glfwWND = window.GetWindow();

    using clock = std::chrono::high_resolution_clock;
    auto lastFrameTime = clock::now();
    ImGuiIO& IO = ImGui::GetIO();

    while (!glfwWindowShouldClose(glfwWND))
    {
        auto now = clock::now();
        std::chrono::duration<float> delta = now - lastFrameTime;
        float deltaTime = delta.count();
        lastFrameTime = now;

        EngineDoFrame(&window, deltaTime);

        glfwPollEvents();
    }

    return 0;
}

Instance& Engine::AddAMesh(const std::string& Path, const std::string& Name,
    FLOAT3 pos, FLOAT3 Size, bool Selec)
{
    auto obj = std::make_unique<Object>(
        Name,
        1,
        pos,
        Size,
        INT3(
            static_cast<int>(Color3.x * 255.0f),
            static_cast<int>(Color3.y * 255.0f),
            static_cast<int>(Color3.z * 255.0f)
        )
    );
    obj->UniqueID = Index;
#if DIRECTX11 == 1
    obj->OBJmesh.Load(assets + Path, window.GetGraphics().GetDevice());
#endif
    obj->Selected = Selec;

    Object* objPtr = obj.get();
    Drawables.push_back(std::move(obj));

    Index++;
    return *objPtr;
}

void Engine::EngineDoFrame(Window* wnd, float deltatime)
{
    if (ImGui::GetCurrentContext() == nullptr) {
        std::cerr << "ERROR: No ImGui context set!" << std::endl;
        return;
    }

    Graphics& graphics = wnd->GetGraphics();

#if INEDITOR == 1
    if (ImGuiInited) {
        #if DIRECTX11 == 1
            ImGui_ImplDX11_NewFrame();
        #endif
        
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
#else
    graphics.SetRenderTargetToBackBuffer();
#endif
    graphics.ClearBuffer(0.0f, 0.0f, 1.0f);

    static bool CubeB = false;
    int width, height;
    glfwGetFramebufferSize(wnd->GetWindow(), &width, &height);

    HWND hwnd = glfwGetWin32Window(wnd->GetWindow());
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.DisplaySize = ImVec2((float)width, (float)height);

    int window_width, window_height;
    glfwGetWindowSize(wnd->GetWindow(), &window_width, &window_height);

    io.DisplayFramebufferScale = ImVec2(
        window_width > 0 ? (float)width / (float)window_width : 1.0f,
        window_height > 0 ? (float)height / (float)window_height : 1.0f
    );

    if (screen_width != width || screen_height != height) {
        wnd->GetGraphics().ReSizeWindow(width, height, hwnd);
        screen_width = width;
        screen_height = height;
        std::cout << "Screen resized to: " << screen_width << "x" << screen_height << std::endl;
    }

    if (!CubeB) {
        window.GetGraphics().ReSizeWindow(screen_width, screen_height, hwnd);
        AddAMesh("\\Cube.obj", "TestCube", { 1,1,1 }, { 1,1,1 }, false);

        
        CubeB = true;
    }

#if VULKAN == 1
    static float orien = 0.0f;
    orien += 0.00001f;
    FLOAT3 Orientation3 = { 0,orien , 0 };
    FLOAT3 posParam = { 0, 0, 0 };

    FLOAT3 scaleParam = { 1, 2, 1 };

    INT3 colorParam = { 0, 0, 0 };

    wnd->GetGraphics().VR.get()->AddAMesh(deltatime, Orientation3, posParam, scaleParam, colorParam, scaleParam, false, 1.0f, 1.0f, 1);
#endif
    
    bool ctrlPressed = (glfwGetKey(wnd->GetWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);
    if (ctrlPressed) {
        AddAMesh("\\Cube.obj", "TestCube", { 1,1,1 }, { 1,1,1 }, false);
    }

#if INEDITOR == 1
    graphics.SetRenderTargetToScene();

    graphics.ClearSceneBuffer(0.1f, 0.2f, 0.3f);
#endif
    
    graphics.DrawAFrame(deltatime, Drawables);

#if INEDITOR == 1
    graphics.SetRenderTargetToBackBuffer();
#endif

#if INEDITOR == 1
    if (ImGuiInited) {
        makeGui.MakeIMViewPort(*wnd);

        makeGui.MakeIMGui(
            *wnd,
            Drawables,
            [this](const std::string& path, const std::string& name,
                FLOAT3 pos, FLOAT3 size, bool Selec) -> Instance*
            {
                return &AddAMesh(path, name, pos, size, Selec);
            },
            reinterpret_cast<float*>(&Color3),
            false
        );
    }
#endif

    CameraControl camC;
    if (!ctrlPressed)
        camC.MakeCameraControls(*wnd, deltatime);
#if INEDITOR == 1 
    if (ImGuiInited) {
        ImGui::Render();

        #if DIRECTX11 == 1
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        #endif
    }
#endif

    wnd->GetGraphics().EndFrame();
}