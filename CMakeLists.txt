cmake_minimum_required(VERSION 3.10)
project(UntitledGameEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------- Vulkan ----------------
find_package(Vulkan REQUIRED)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found.\nInstall it from https://vulkan.lunarg.com/")
endif()

message(STATUS "Using Vulkan SDK at: ${Vulkan_INCLUDE_DIRS}")

#GLFW
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

#ImGui
set(IMGUI_SOURCES
    Core/Libs/ImGui/imgui.cpp
    Core/Libs/ImGui/imgui_draw.cpp
    Core/Libs/ImGui/imgui_tables.cpp
    Core/Libs/ImGui/imgui_widgets.cpp
    Core/Libs/ImGui/backends/imgui_impl_glfw.cpp
    Core/Libs/ImGui/backends/imgui_impl_dx11.cpp
)

#Definitions
add_definitions(-DPROJECT_DIR="${CMAKE_SOURCE_DIR}/")

#Executable
add_executable(UntitledGameEngine
    main.cpp
    Core/SetupPaths/MakeFiles.cpp
    Core/Camera/Camera.cpp
    Core/ErrorHandling/Expectation/Expection.cpp
    Core/Graphics/Graphics.cpp
    Core/Instances/Instances/Mesh/DX11/MeshDX11.cpp
    Core/Gui/ImGuiManager.cpp
    Core/Gui/MakeGui.cpp
    Core/Misc/Colission/Colission.cpp
    Core/Misc/Controller/Controller.cpp
    Core/Misc/Timer/Timer.cpp
    Core/Variables/GLOBALS.cpp
    Editor/Camera/CameraControl.cpp
    Editor/Services/Raycasting.cpp
    Core/Runtime/Engine.cpp
    Core/Window/Window.cpp
    Core/Graphics/Dx11/Dx11Renderer.cpp
    Core/Graphics/Vulkan/VulkanRender.cpp
    Core/Graphics/Vulkan/ScoreCounter.cpp
    ${IMGUI_SOURCES}
)

# ---------------- Include dirs ----------------
target_include_directories(UntitledGameEngine PRIVATE
    include
    Core
    Core/Libs/Assimp/include
    Core/Libs/SDL3
    Core/Libs/ImGui
    Core/Libs/ImGui/backends
    Core/Instances/Instances/

    ${Vulkan_INCLUDE_DIRS}
)

# ---------------- Link libraries ----------------
target_link_libraries(UntitledGameEngine PRIVATE
    ${CMAKE_SOURCE_DIR}/Libs/assimp-vc143-mt.lib
    ${CMAKE_SOURCE_DIR}/Libs/SDL3.lib
    Vulkan::Vulkan
    glfw
)

if(WIN32)
    target_link_libraries(UntitledGameEngine PRIVATE gdi32 opengl32)
    target_link_options(UntitledGameEngine PRIVATE "/SUBSYSTEM:CONSOLE")
endif()

# ---------------- Post-build DLL copy ----------------
if(WIN32)
    set(DLLS
        ${CMAKE_SOURCE_DIR}/Libs/assimp-vc143-mt.dll
        ${CMAKE_SOURCE_DIR}/Libs/SDL3.dll
    )

    foreach(dll ${DLLS})
        add_custom_command(TARGET UntitledGameEngine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${dll}
            $<TARGET_FILE_DIR:UntitledGameEngine>
        )
    endforeach()
endif()

# ---------------- Shader files copy ----------------
set(SHADER_FILES
    ${CMAKE_SOURCE_DIR}/Core/Shaders/VertexShader.hlsl
    ${CMAKE_SOURCE_DIR}/Core/Shaders/PixelShader.hlsl
)

foreach(shader ${SHADER_FILES})
    add_custom_command(TARGET UntitledGameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${shader}
        $<TARGET_FILE_DIR:UntitledGameEngine>
    )
endforeach()

# ---------------- Compile HLSL shaders ----------------
if(WIN32)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/VertexShader.cso
        COMMAND fxc.exe /T vs_5_0 /E main /Fo "${CMAKE_CURRENT_BINARY_DIR}/VertexShader.cso"
                "${CMAKE_SOURCE_DIR}/Core/Shaders/VertexShader.hlsl"
        DEPENDS "${CMAKE_SOURCE_DIR}/Core/Shaders/VertexShader.hlsl"
        COMMENT "Compiling Vertex Shader"
    )
    
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/PixelShader.cso
        COMMAND fxc.exe /T ps_5_0 /E main /Fo "${CMAKE_CURRENT_BINARY_DIR}/PixelShader.cso"
                "${CMAKE_SOURCE_DIR}/Core/Shaders/PixelShader.hlsl"
        DEPENDS "${CMAKE_SOURCE_DIR}/Core/Shaders/PixelShader.hlsl"
        COMMENT "Compiling Pixel Shader"
    )
endif()

# ---------------- MSVC tweaks ----------------
if(MSVC)
    set(CMAKE_VS_NO_COMPILE_BATCHING ON)
endif()
