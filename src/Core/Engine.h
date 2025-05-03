#pragma once
#include <chrono>

#include "pch.h"

#include "Core/Window.h"
#include "Core/InputSystem.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/SwapChain.h"
#include "Scene/Scene.h"
#include "Rendering/FrameRenderer.h"

class Engine {
public:
    inline static Engine& getInstance() {
        static Engine instance;
        return instance;
    }

    Engine(const Engine&) = delete;

    Engine& operator=(const Engine&) = delete;

    void init(const std::vector<std::string>& commandLineArguments);

    void runMainLoop();

    inline Window& getWindow() const {
        return *mWindow;
    }

    inline InputSystem& getInputSystem() const {
        return *mInputSystem;
    }

    inline VulkanContext& getVulkanContext() const {
        return *mVulkanContext;
    }

    inline SwapChain& getSwapChain() const {
        return *mSwapChain;
    }

    inline Scene& getScene() const {
        return *mScene;
    }

    inline FrameRenderer& getFrameRenderer() {
        return *mFrameRenderer;
    }

    inline double getDelta() const {
        return mDelta;
    }

private:
    std::vector<std::string> mLaunchArguments;

    std::unique_ptr<Window> mWindow;
    std::unique_ptr<InputSystem> mInputSystem;
    std::unique_ptr<VulkanContext> mVulkanContext;
    std::unique_ptr<SwapChain> mSwapChain;
    std::unique_ptr<Scene> mScene;
    std::unique_ptr<FrameRenderer> mFrameRenderer;

    std::chrono::steady_clock::time_point mFrameStart;
    double mDelta = 0.0;

private:
    Engine() = default;

    void initScene1();

    void initScene2();

    void initScene3();

    void initScene4();

    void onWindowResize(uint32_t width, uint32_t height);

    void prepareFrame();

    void updateTransforms();

    void updateBehaviour();

    void renderFrame();
};
