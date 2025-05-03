#include "pch.h"
#include "Window.h"

Window::Window(const std::string& title) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    mWindowHandle = glfwCreateWindow(800, 600, title.c_str(), nullptr, nullptr);

    assertm(mWindowHandle != nullptr, "Failed to create a window");

    int width, height;
    glfwGetFramebufferSize(mWindowHandle, &width, &height);
    mWidth = width;
    mHeight = height;

    LOG(INFO) << "Window created. Width = " << mWidth << ", height = " << mHeight;

    glfwSetWindowUserPointer(mWindowHandle, this);
    glfwSetFramebufferSizeCallback(mWindowHandle, framebufferResizeCallback);
}

Window::~Window() {
    glfwDestroyWindow(mWindowHandle);
}

std::vector<std::string> Window::getRequiredInstanceExtensions() const {
    std::vector<std::string> requiredExtensions;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (int i = 0; i < glfwExtensionCount; ++i) {
        requiredExtensions.emplace_back(glfwExtensions[i]);
    }

    return requiredExtensions;
}

std::unique_ptr<vk::raii::SurfaceKHR> Window::createSurface(const vk::raii::Instance& instance) const {
    VkSurfaceKHR surface;

    if (glfwCreateWindowSurface(*instance, mWindowHandle, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a surface");
    }

    return std::make_unique<vk::raii::SurfaceKHR>(instance, surface);
}

void Window::framebufferResizeCallback(GLFWwindow* windowHandle, int width, int height) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
    window->mWidth = width;
    window->mHeight = height;
    LOG(INFO) << "Window resized. Width = " << width << ", height = " << height;
    if (window->mResizeHandler) {
        window->mResizeHandler(width, height);
    }
}

void Window::setResizeHandler(WindowResizeHandler&& resizeHandler) {
    mResizeHandler = resizeHandler;
}

void Window::setFullscreen(bool fullscreen) {
    if (fullscreen) {
        int count = 0;
        GLFWmonitor** monitor = glfwGetMonitors(&count);
        const GLFWvidmode* mode = glfwGetVideoMode(*monitor);

        glfwSetWindowMonitor(mWindowHandle, *monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(mWindowHandle, nullptr, 50, 50, 800, 600, 0);
    }
}
