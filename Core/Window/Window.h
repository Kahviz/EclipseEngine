#pragma once

#include <GLFW/glfw3.h>
#include <memory>
#include <Graphics/Graphics.h>

class Window
{
public:
    Window(int Height, int Width, std::string Name);
    void SetWindowIcon(GLFWwindow* window);
    ~Window();
    GLFWwindow* GetWindow() const noexcept;
    Graphics& GetGraphics();

private:
    bool Inited = false;
    GLFWwindow* m_window = nullptr;
    std::unique_ptr<Graphics> pGfx;
};
