cmake_minimum_required(VERSION 3.10)
project(UntitledGameEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(MSVC)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<CONFIG:Debug,RelWithDebInfo>,ProgramDatabase,Embedded>")
    
    add_compile_options($<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:/Zi>)
    
    add_compile_options($<$<CONFIG:Debug>:/Od>)
    
    add_compile_options($<$<CONFIG:Debug>:/RTC1>)
    
    add_link_options($<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:/DEBUG:FULL>)
endif()

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
    Core/Libs/ImGui/backends/imgui_impl_vulkan.cpp
)

#Definitions
add_definitions(
    -DPROJECT_DIR="${CMAKE_SOURCE_DIR}/"
)

#Executable
add_executable(UntitledGameEngine
    ${IMGUI_SOURCES}
    main.cpp
    Core/SetupPaths/MakeFiles.cpp
    Core/Camera/Camera.cpp
    Core/ErrorHandling/Expectation/Expection.cpp
    Core/Graphics/Graphics.cpp
    Core/Instances/Instances/Mesh/DX11/MeshDX11.cpp
    Core/Instances/Instances/Mesh/Vulkan/MeshVulkan.cpp
    Core/Gui/ImGuiManager.cpp
    Core/Misc/Colission/Colission.cpp
    Core/Misc/Controller/Controller.cpp
    Core/Misc/Timer/Timer.cpp
    Core/Variables/GLOBALS.cpp
    Core/Services/Raycasting.cpp
    Core/Runtime/Engine.cpp
    Core/Window/Window.cpp
    Core/Graphics/Dx11/Dx11Renderer.cpp
    Core/Graphics/Vulkan/VulkanRender.cpp
    Core/Graphics/Vulkan/ScoreCounter.cpp
    Core/Debugging/Profiler/Profiler.cpp
    Core/Debugging/Functions/UGE_ASSERTS.h
    Core/Libs/ImGui/imgui_demo.cpp
    Core/Graphics/Texture/Texture.h 
    Core/Graphics/Texture/Texture.cpp 

    #Editor
    Editor/Gui/MakeGui.cpp
    Editor/Camera/CameraControl.cpp

    #Math
    include/Math/M_Utils.h
    include/Math/StoreFloat.h
    include/Math/MatrixLookAtLH.h
    include/Math/TransformNormal.h
    include/Math/LoadINT.h
    include/Math/Variables.h 
    include/Math/FLOAT2.h
 "UntilitedGameEngine.h" "Core/Multiplatform/UntilitedPlatform.h" "Core/UntilitedPhysics/Gravity.h")

target_include_directories(UntitledGameEngine PRIVATE
    include
    Core
    Editor/
    ../Editor/
    Core/Libs/Assimp/include
    Core/Libs/SDL3
    Core/Libs/ImGui
    Core/Libs/ImGui/backends
    Core/Instances/Instances/
    Core/Debugging/Functions/
    Core/Graphics/Texture/
    Editor/Camera/
    Core/Services/
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(UntitledGameEngine PRIVATE
    ${CMAKE_SOURCE_DIR}/Libs/assimp-vc143-mt.lib
    ${CMAKE_SOURCE_DIR}/Libs/SDL3.lib
    Vulkan::Vulkan
    glfw
)

if(WIN32)
    target_link_options(UntitledGameEngine PRIVATE "/SUBSYSTEM:CONSOLE")
endif()

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

set(SHADER_FILES
    ${CMAKE_SOURCE_DIR}/Core/Shaders/VertexShader.hlsl
    ${CMAKE_SOURCE_DIR}/Core/Shaders/PixelShader.hlsl
    ${CMAKE_SOURCE_DIR}/Core/Shaders/PixelShaderTexture.hlsl
)

foreach(shader ${SHADER_FILES})
    add_custom_command(TARGET UntitledGameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${shader}
        $<TARGET_FILE_DIR:UntitledGameEngine>
    )
endforeach()

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

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/PixelShaderTexture.cso
        COMMAND fxc.exe /T ps_5_0 /E main /Fo "${CMAKE_CURRENT_BINARY_DIR}/PixelShaderTexture.cso"
                "${CMAKE_SOURCE_DIR}/Core/Shaders/PixelShaderTexture.hlsl"
        DEPENDS "${CMAKE_SOURCE_DIR}/Core/Shaders/PixelShaderTexture.hlsl"
        COMMENT "Compiling PixelTexture Shader"
    )
endif()

if(MSVC)
    set(CMAKE_VS_NO_COMPILE_BATCHING ON)
endif()
