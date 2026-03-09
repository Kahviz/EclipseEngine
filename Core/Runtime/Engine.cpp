#include "Engine.h"
#include <stdexcept>
#include "chrono"
#include <iostream>
#include <Object.h>
#include <GLOBALS.h>
#include "../Editor/Camera/CameraControl.h"
#include "UGE_ASSERTS.h"

#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_vulkan.h"

Engine::Engine()
    : window(1280, 800, "UntilitedGameEngine")
{
    if (!ImGuiInited) {
        std::cout << "ImGui version: " << IMGUI_VERSION << std::endl;
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
#else  // VULKAN
        VulkanRender* vr = window.GetGraphics().VR.get();

        VkRenderPass renderPass = vr->GetRenderPass();
        if (renderPass == VK_NULL_HANDLE) {
            throw std::runtime_error("Render pass is null!");
        }

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.ApiVersion = VK_API_VERSION_1_2;
        init_info.Instance = vr->GetVulkanInstance();
        init_info.PhysicalDevice = vr->GetPhysicalDevice();
        init_info.Device = vr->GetDevice();
        init_info.QueueFamily = vr->GetGraphicsFamilyIndex();
        init_info.Queue = vr->GetGraphicsQueue();
        init_info.DescriptorPool = vr->GetImGuiPool();
        init_info.MinImageCount = 2;
        init_info.ImageCount = static_cast<uint32_t>(vr->GetSwapChainImageViews().size());
        init_info.PipelineCache = VK_NULL_HANDLE;

        init_info.PipelineInfoMain.RenderPass = renderPass;
        init_info.PipelineInfoMain.Subpass = 0;

        init_info.PipelineInfoForViewports = init_info.PipelineInfoMain;
        init_info.UseDynamicRendering = false;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = [](VkResult err) {
            if (err != VK_SUCCESS) {
                std::cerr << "Vulkan Error: " << err << std::endl;
            }
        };

        if (!ImGui_ImplVulkan_Init(&init_info)) {
            throw std::runtime_error("Failed to initialize ImGui Vulkan backend");
        }

        ImGuiInited = true;
#endif
        ImGuiIO& io = ImGui::GetIO();
        std::string fontPath = fonts + "\\RobotoFont.ttf";
        ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);

        if (font == nullptr) {
            std::cout << "Could not load font, using default font" << std::endl;
            io.Fonts->AddFontDefault();
        }

        #ifdef _DEBUG
            std::cout << "ImGui initialized successfully!" << std::endl;
        #endif

        window.SetWindowIcon(window.GetWindow());
        ImGuiIO& IO = ImGui::GetIO();
        IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        IO.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;

    }
}
Engine::~Engine()
{
    if (ImGuiInited) {
        #if DIRECTX11 == 1
            ImGui_ImplDX11_Shutdown();
        #endif

        #if VULKAN == 1
            ImGui_ImplVulkan_Shutdown();
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

    profiler.PrintInformation();

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
    obj->OBJmesh.DM.Load(assets + Path, window.GetGraphics().GetDevice());

    #ifdef _DEBUG
        std::cout << "DX11 Mesh loaded: " << assets + Path << std::endl;
        std::cout << "Vertices: " << obj->OBJmesh.GetVertices().size() << std::endl;
        std::cout << "Indices: " << obj->OBJmesh.GetIndices().size() << std::endl;
    #endif
#endif

#if VULKAN == 1
    obj.get()->OBJmesh.VM.Load(
        assets + Path,
        window.GetGraphics().GetDevice(),
        window.GetGraphics().GetPhysicalDevice(),
        window.GetGraphics().VR.get()->GetCommandPool(),
        window.GetGraphics().VR.get()->GetGraphicsQueue()
    );
#endif

#ifdef _DEBUG
    std::cout << "Loading mesh: " << assets + Path << std::endl;

    #if VULKAN == 1
        std::cout << "Vertices: " << obj.get()->OBJmesh.VM.GetVertices().size() << ", Indices: " << obj.get()->OBJmesh.VM.GetIndices().size() << std::endl;
    #endif
#endif
    obj->Selected = Selec;

    std::string fullPath = assets + "\\Textures\\TestTexture.png";
    std::cout << fullPath << std::endl;

#if DIRECTX11 == 1
    obj->texture.Load(fullPath, *window.GetGraphics().DR.get());
#endif
    Instance* objPtr = obj.get();
    Drawables.push_back(std::move(obj));

    Index++;
    return *objPtr;
}

void ScreenResizerDetector(Window* wnd) {
    static int lastWidth = 0, lastHeight = 0;
    glfwGetFramebufferSize(wnd->GetWindow(), &screen_width, &screen_height);

    if (screen_width != lastWidth || screen_height != lastHeight) {
        wnd->GetGraphics().ReSizeWindow(screen_width, screen_height, wnd);
        lastWidth = screen_width;
        lastHeight = screen_height;
#ifdef _DEBUG
    std::cout << "Screen resized to: " << screen_width << "x" << screen_height << std::endl;
#endif
    }

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)screen_width, (float)screen_height);

    int window_width, window_height;
    glfwGetWindowSize(wnd->GetWindow(), &window_width, &window_height);
    io.DisplayFramebufferScale = ImVec2(
        window_width > 0 ? (float)screen_width / (float)window_width : 1.0f,
        window_height > 0 ? (float)screen_height / (float)window_height : 1.0f
    );
}

void Engine::EngineDoFrame(Window* wnd, float deltatime)
{
    if (ImGui::GetCurrentContext() == nullptr) {
        std::cerr << "ERROR: No ImGui context set!" << std::endl;
        return;
    }

    static bool CubeB = false;

    Graphics& graphics = wnd->GetGraphics();

#if VULKAN == 1
    VulkanRender* vr = graphics.VR.get();
#endif
    ScreenResizerDetector(wnd);

#if INEDITOR == 1
    if (ImGuiInited) {
        #if DIRECTX11 == 1
            ImGui_ImplDX11_NewFrame();
        #endif

        #if VULKAN == 1
            ImGui_ImplVulkan_NewFrame();
        #endif
        
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
#endif

    graphics.ClearBuffer(0.0f, 0.0f, 1.0f);

    if (InProject) {
        if (!CubeB) {
            AddAMesh("\\Cube.obj", "Cube", { 0,0,0 }, { 0.5,1,0.5 }, false);
            wnd->GetGraphics().GetCamera().SetPosition({ 5,5,5 });
            wnd->GetGraphics().GetCamera().SetRotation({ 0.625999,3.926,0 });
            CubeB = true;
        }
    }

    bool ctrlPressed = (glfwGetKey(wnd->GetWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);

    if (InProject) {
        if (ctrlPressed) {
            // AddAMesh("\\Cylinder.obj", "TestCylinder", { 0,0,0 }, { 0.5,1,0.5 }, false);
        }
    }

#if INEDITOR == 1
    if (ImGuiInited) {
        if (InProject) {
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
        else {
            if (makeGui.MakeDashBoard()) {
                InProject = true;
            }
        }
    }
#endif

#if VULKAN == 1
    for (auto& Drawableptr : Drawables) {
        auto Drawable = Drawableptr.get();
        if (Drawable->CanDraw()) {
            wnd->GetGraphics().RenderAMesh(
                deltatime,
                Drawable,
                Drawable->Orientation,
                Drawable->pos,
                Drawable->Size,
                Drawable->color,
                Drawable->Velocity,
                Drawable->Anchored,
                1.0f,
                1.0f,
                Drawable->UniqueID
            );
        }
    }
#endif

#if DIRECTX11 == 1
    wnd->GetGraphics().DrawAFrame(deltatime, Drawables);
#endif

    CameraControl camC;
    if (!ctrlPressed && !Typing)
        camC.MakeCameraControls(*wnd, deltatime);

#if INEDITOR == 1 
    if (ImGuiInited) {
        ImGui::Render();
        #if DIRECTX11 == 1
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        #endif
        //Vulkan Does it in the renderer
    }
#endif
    float fps = 1.0f / deltatime;
    profiler.AddFPS(static_cast<int>(fps));

#if VULKAN == 1
    wnd->GetGraphics().DrawAFrame(deltatime, Drawables);
#endif

    wnd->GetGraphics().EndFrame();
}