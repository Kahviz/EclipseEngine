@echo off
setlocal enabledelayedexpansion

echo Compiling Vulkan shaders

set "shaders[0]=vertex.glsl vert vertex.spv"
set "shaders[1]=fragment.glsl frag fragment.spv"

set count=2

for /l %%i in (0,1,%count%) do (
    for /f "tokens=1,2,3" %%a in ("!shaders[%%i]!") do (
        set "input=%%a"
        set "type=%%b"
        set "output=%%c"
        
        if exist "Core\Shaders\Source\!input!" (
            echo Compiling !input!...
            "C:\VulkanSDK\1.4.335.0\Bin\glslangValidator.exe" -V "Core\Shaders\Source\!input!" -o "Core\Shaders\!output!" --target-env vulkan1.2 -S !type!
            if errorlevel 1 (
                echo ERROR: Failed to compile !input!
                pause
                exit /b 1
            ) else (
                echo !output! created
            )
        ) else (
            echo ERROR: !input! not found
            dir Core\Shaders\Source\
            pause
            exit /b 1
        )
    )
)

echo Vulkan Shaders compiled successfully
pause