cmake_minimum_required(VERSION 3.10)
project(EclipseEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

##How to add a new shader to DX11
##You just need add it's properties to SHADER_FILES, SHADER_NAMES, SHADER_TYPES,SHADER_FILES

if(MSVC)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<CONFIG:Debug,RelWithDebInfo>,ProgramDatabase,Embedded>")
    
    add_compile_options($<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:/Zi>)
    
    add_compile_options($<$<CONFIG:Debug>:/Od>)
    
    add_compile_options($<$<CONFIG:Debug>:/RTC1>)
    
    add_link_options($<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:/DEBUG:FULL>)
endif()

find_package(Vulkan REQUIRED)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found.\nInstall it from https://vulkan.lunarg.com/ or just goto GLOBALS.h and put VULKAN = 0 and DIRECTX11 = 1")
endif()

message(STATUS "Using Vulkan SDK at: ${Vulkan_INCLUDE_DIRS}")

#GLFW
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_Declare(
    EclipseMathLibrary
    UntilitedMathLibrary
    GIT_REPOSITORY https://github.com/Kahviz/EclipseMathLibrary
    GIT_TAG master
)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(EclipseMathLibrary)

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
add_executable(EclipseEngine
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
    Core/UntilitedPhysics/Physics.cpp
    Core/Multiplatform/UntilitedPlatform.h
    Core/UntilitedPhysics/Gravity.h
    Core/UntilitedPhysics/Physics.h
    #Editor
    Editor/Gui/MakeGui.cpp
    Editor/Camera/CameraControl.cpp

    UntilitedGameEngine.h

    #VulkanRenderer Subclasses
    Core/Graphics/Vulkan/VulkanDevice/VulkanDevice.h
    Core/Graphics/Vulkan/VulkanDevice/VulkanDevice.cpp
    Core/Graphics/Vulkan/VulkanInstance/VulkanInstance.h
    Core/Graphics/Vulkan/VulkanInstance/VulkanInstance.cpp
    Core/Graphics/Vulkan/VulkanSwapchain/VulkanSwapchain.h
    Core/Graphics/Vulkan/VulkanSwapchain/VulkanSwapchain.cpp
    Core/Graphics/Vulkan/VulkanCommandBuffer/VulkanCommandBuffer.h
    Core/Graphics/Vulkan/VulkanCommandBuffer/VulkanCommandBuffer.cpp
    Core/Graphics/Vulkan/VulkanPipeline/VulkanPipeline.h
    Core/Graphics/Vulkan/VulkanPipeline/VulkanPipeline.cpp
)

target_include_directories(EclipseEngine PRIVATE
    include
    Core
    Editor/
    ../Editor/
    Core/Libs/Assimp/include
    Core/Libs/SDL3
    Core/Libs/ImGui
    Core/Libs/ImGui/backends
    Core/Instances/Instances/
    Core/Instances/
    Core/Debugging/Functions/
    Core/Graphics/Texture/
    Editor/Camera/
    Core/Services/
    Libs/STBIcons/
    ${Vulkan_INCLUDE_DIRS}
    ${eclipsemathlibrary_SOURCE_DIR}
)

target_link_libraries(EclipseEngine PRIVATE
    ${CMAKE_SOURCE_DIR}/Libs/assimp-vc143-mt.lib
    ${CMAKE_SOURCE_DIR}/Libs/SDL3.lib
    Vulkan::Vulkan
    glfw
)

if(WIN32)
    target_link_options(EclipseEngine PRIVATE "/SUBSYSTEM:CONSOLE")
endif()


if(WIN32)
    set(DLLS
        ${CMAKE_SOURCE_DIR}/Libs/assimp-vc143-mt.dll
        ${CMAKE_SOURCE_DIR}/Libs/SDL3.dll
    )

    foreach(dll ${DLLS})
        add_custom_command(TARGET EclipseEngine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${dll}
            $<TARGET_FILE_DIR:EclipseEngine>
        )
    endforeach()
endif()

#Shaders
set(SHADER_FILES
    VertexShader.hlsl
    PixelShader.hlsl
    PixelShaderTexture.hlsl
)

set(SHADER_TYPES
    vs_5_0
    ps_5_0
    ps_5_0
)


# Kopioi HLSL-tiedostot build-kansioon (joka kerta)
foreach(shader_file ${SHADER_FILES})
    set(source_file "${CMAKE_SOURCE_DIR}/Core/Shaders/${shader_file}")
    
    add_custom_command(TARGET EclipseEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${source_file}
        $<TARGET_FILE_DIR:EclipseEngine>
        COMMENT "Copying ${shader_file}"
    )
endforeach()

if(WIN32)
    # Käännä shaderit CSO-tiedostoiksi
    list(LENGTH SHADER_FILES SHADER_COUNT)
    
    foreach(index RANGE ${SHADER_COUNT})
        math(EXPR current_index "${index} - 1")
        
        if(current_index GREATER -1 AND current_index LESS ${SHADER_COUNT})
            list(GET SHADER_FILES ${current_index} shader_file)
            list(GET SHADER_TYPES ${current_index} shader_type)
            
            string(REPLACE ".hlsl" "" shader_name ${shader_file})
            
            set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${shader_name}.cso")
            set(input_file "${CMAKE_SOURCE_DIR}/Core/Shaders/${shader_file}")
            
            add_custom_command(
                OUTPUT ${output_file}
                COMMAND ${CMAKE_COMMAND} -E remove -f ${output_file}
                COMMAND fxc.exe /T ${shader_type} /E main /Fo "${output_file}" "${input_file}"
                DEPENDS ${input_file}
                COMMENT "Compiling ${shader_file} -> ${shader_name}.cso"
            )
            
            list(APPEND GENERATED_SHADERS ${output_file})
            
            add_custom_command(TARGET EclipseEngine POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${output_file}
                $<TARGET_FILE_DIR:EclipseEngine>
                COMMENT "Copying ${shader_name}.cso"
            )
        endif()
    endforeach()
    
    if(GENERATED_SHADERS)
        add_custom_target(CompileShaders DEPENDS ${GENERATED_SHADERS})
        add_dependencies(EclipseEngine CompileShaders)
    endif()
endif()

if(MSVC)
    set(CMAKE_VS_NO_COMPILE_BATCHING ON)
endif()
