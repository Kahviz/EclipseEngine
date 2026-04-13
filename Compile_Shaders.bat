@echo off

echo Compiling Vulkan shaders

REM vertex shader
if exist "Core\Shaders\Source\vertex.glsl" (
    echo Compiling vertex.glsl...
    "C:\VulkanSDK\1.4.335.0\Bin\glslangValidator.exe" -V "Core\Shaders\Source\vertex.glsl" -o "Core\Shaders\vertex.spv" --target-env vulkan1.2 -S vert
    if errorlevel 1 (
        echo ERROR: Failed to compile vertex.glsl
        pause
        exit /b 1
    )
)

REM fragment shader
if exist "Core\Shaders\Source\fragment.glsl" (
    echo Compiling fragment.glsl...
    "C:\VulkanSDK\1.4.335.0\Bin\glslangValidator.exe" -V "Core\Shaders\Source\fragment.glsl" -o "Core\Shaders\fragment.spv" --target-env vulkan1.2 -S frag
    if errorlevel 1 (
        echo ERROR: Failed to compile fragment.glsl
        pause
        exit /b 1
    )
)

REM shadowvertex shader
if exist "Core\Shaders\Source\shadow_vertex.glsl" (
    echo Compiling shadow_vertex.glsl...
    "C:\VulkanSDK\1.4.335.0\Bin\glslangValidator.exe" -V "Core\Shaders\Source\shadow_vertex.glsl" -o "Core\Shaders\shadow_vertex.spv" --target-env vulkan1.2 -S vert
    if errorlevel 1 (
        echo ERROR: Failed to compile shadow_vertex.glsl
        pause
        exit /b 1
    )
)
echo Vulkan Shaders compiled successfully
pause