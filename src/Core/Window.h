#pragma once
#include "pch.h"

#include <GLFW/glfw3.h>

typedef std::function<void(uint32_t width, uint32_t height)> WindowResizeHandler;

class Window {
public:
    explicit Window(const std::string& title);

    Window(const Window&) = delete;

    Window& operator=(const Window&) = delete;

    ~Window();

    inline std::tuple<uint32_t, uint32_t> getSize() const {
        return {mWidth, mHeight};
    };

    inline uint32_t getWidth() const {
        return mWidth;
    };

    uint32_t getHeight() const {
        return mHeight;
    };

    inline GLFWwindow* operator*() const {
        return mWindowHandle;
    }

    std::vector<std::string> getRequiredInstanceExtensions() const;

    std::unique_ptr<vk::raii::SurfaceKHR> createSurface(const vk::raii::Instance& instance) const;

    void setResizeHandler(WindowResizeHandler&& resizeHandler);

    void setFullscreen(bool fullscreen);

private:
    GLFWwindow* mWindowHandle;

    uint32_t mWidth;
    uint32_t mHeight;

    WindowResizeHandler mResizeHandler;

private:
    static void framebufferResizeCallback(GLFWwindow* windowHandle, int width, int height);
};
