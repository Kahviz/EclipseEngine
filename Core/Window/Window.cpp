#include "Window.h"
#include "memory.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

#include "imgui.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_dx11.h>

#include "GLOBALS.h"

#define STB_IMAGE_IMPLEMENTATION
#include <Libs/STBIcons/stb_image.h>

Window::Window(int Width, int Height, std::string Name)
{
    if (!Inited) {
        if (!glfwInit())
            throw std::runtime_error("Failed to initialize GLFW");
        Inited = true;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(Width, Height, Name.c_str(), nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    pGfx = std::make_unique<Graphics>();

    if (!pGfx.get()->InitGraphics(m_window)) {
        MakeAError("Graphics Initing Error!");

        throw std::runtime_error("Failed to initialize graphics!");
    }
}


void Window::SetWindowIcon(GLFWwindow* window) {
    GLFWimage images[1];
    int width, height, channels;

    std::string fullPath = appData + "/UntilitedGameEngine/Icons/UntilitedIcon.png";
    unsigned char* pixels = stbi_load(fullPath.c_str(), &width, &height, &channels, 4);

    if (pixels) {
        GLFWimage images[1];
        images[0].width = width;
        images[0].height = height;
        images[0].pixels = pixels;

        glfwSetWindowIcon(window, 1, images);
        stbi_image_free(pixels);
    }
}

Window::~Window()
{
    if (m_window) {
        glfwDestroyWindow(m_window);
    }

    if (Inited) {
        glfwTerminate();
        Inited = false;
    }
}

GLFWwindow* Window::GetWindow() const noexcept
{
    return m_window;
}

Graphics& Window::GetGraphics()
{
    return *pGfx;
}