cmake_minimum_required(VERSION 3.10)

project(UntilitedGameEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VULKAN_SDK_PATH "C:/VulkanSDK/1.4.335.0")  # SINUN polkusi

if(NOT EXISTS "${VULKAN_SDK_PATH}")
    message(FATAL_ERROR "Vulkan SDK not found at: ${VULKAN_SDK_PATH}")
endif()

message(STATUS "Using Vulkan SDK at: ${VULKAN_SDK_PATH}")

# 2. VULKAN HEADERIT JA KIRJASTOT
set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib/vulkan-1.lib")

# Hot Reload (MSVC)
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# ---- GLFW ----
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

set(IMGUI_SOURCES
    Core/Libs/ImGui/imgui.cpp
    Core/Libs/ImGui/imgui_draw.cpp
    Core/Libs/ImGui/imgui_tables.cpp
    Core/Libs/ImGui/imgui_widgets.cpp
    Core/Libs/ImGui/backends/imgui_impl_glfw.cpp
    Core/Libs/ImGui/backends/imgui_impl_dx11.cpp
)


# ---- Add Executable ----
add_executable(
    UntilitedGameEngine
    main.cpp
    Core/Camera/Camera.cpp
    Core/ErrorHandling/Expectation/Expection.cpp
    Core/Graphics/Graphics.cpp
    Core/Instances/Instances/Mesh/Mesh.cpp
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

# ---- Include directories ----
target_include_directories(UntilitedGameEngine PRIVATE
    include
    Core
    Core/Libs/Assimp/include
    Core/Libs/SDL3/
    Core/Libs/ImGui/
    Core/Libs/ImGui/backends/
    out/build/x64-release/_deps/glfw-src/include/
    Core/Runtime/
    Core/Graphics/
    ${Vulkan_INCLUDE_DIRS}
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(UntilitedGameEngine PRIVATE
    ${CMAKE_SOURCE_DIR}/Libs/assimp-vc143-mt.lib
    ${CMAKE_SOURCE_DIR}/Libs/SDL3.lib
    ${Vulkan_LIBRARIES}
    glfw
)
target_compile_definitions(UntilitedGameEngine PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}/")

if(WIN32)
    target_link_libraries(UntilitedGameEngine PRIVATE
        gdi32
    )
    
    # Kopioi vulkan-1.dll output kansioon
    if(EXISTS "${VULKAN_SDK_PATH}/Bin/vulkan-1.dll")
        add_custom_command(TARGET VulkanTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VULKAN_SDK_PATH}/Bin/vulkan-1.dll"
            $<TARGET_FILE_DIR:VulkanTest>
        )
        message(STATUS "Will copy vulkan-1.dll to output directory")
    endif()
endif()

if(WIN32)
    target_link_libraries(UntilitedGameEngine PRIVATE
        gdi32
    )
    
    # Kopioi vulkan-1.dll output kansioon
    if(EXISTS "${VULKAN_SDK_PATH}/Bin/vulkan-1.dll")
        add_custom_command(TARGET VulkanTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VULKAN_SDK_PATH}/Bin/vulkan-1.dll"
            $<TARGET_FILE_DIR:VulkanTest>
        )
        message(STATUS "Will copy vulkan-1.dll to output directory")
    endif()
endif()


if(WIN32)
    target_link_libraries(UntilitedGameEngine PRIVATE
        opengl32
        gdi32
    )
    # Käytetään konsolia kaikissa build-moodissa
    target_link_options(UntilitedGameEngine PRIVATE "/SUBSYSTEM:CONSOLE")
endif()

# ---- Copy DLLs ----
if(WIN32)
    add_custom_command(TARGET UntilitedGameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/Libs/assimp-vc143-mt.dll"
            $<TARGET_FILE_DIR:UntilitedGameEngine>
    )

    add_custom_command(TARGET UntilitedGameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/Libs/SDL3.dll"
            $<TARGET_FILE_DIR:UntilitedGameEngine>
    )
endif()

# ---- Shader files ----
set(SHADER_FILES
    ${CMAKE_SOURCE_DIR}/Core/Shaders/VertexShader.hlsl
    ${CMAKE_SOURCE_DIR}/Core/Shaders/PixelShader.hlsl
)

foreach(shader ${SHADER_FILES})
    add_custom_command(TARGET UntilitedGameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${shader}
            $<TARGET_FILE_DIR:UntilitedGameEngine>
    )
endforeach()

# ---- Compile HLSL shaders ----
if(WIN32)
    # Vertex shader
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/VertexShader.cso
        COMMAND fxc.exe /T vs_5_0 /E main /Fo "${CMAKE_CURRENT_BINARY_DIR}/VertexShader.cso"
                "${CMAKE_SOURCE_DIR}/Core/Shaders/VertexShader.hlsl"
        DEPENDS "${CMAKE_SOURCE_DIR}/Core/Shaders/VertexShader.hlsl"
        COMMENT "Compiling Vertex Shader"
    )
    
    # Pixel shader
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/PixelShader.cso
        COMMAND fxc.exe /T ps_5_0 /E main /Fo "${CMAKE_CURRENT_BINARY_DIR}/PixelShader.cso"
                "${CMAKE_SOURCE_DIR}/Core/Shaders/PixelShader.hlsl"
        DEPENDS "${CMAKE_SOURCE_DIR}/Core/Shaders/PixelShader.hlsl"
        COMMENT "Compiling Pixel Shader"
    )
    

endif()

if(MSVC)
    set(CMAKE_VS_NO_COMPILE_BATCHING ON)
endif()