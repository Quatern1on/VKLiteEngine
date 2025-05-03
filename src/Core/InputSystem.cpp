#include "pch.h"
#include "InputSystem.h"

#include <GLFW/glfw3.h>

InputSystem::InputSystem(const Window& window)
        : mWindow(std::ref(window)) {
    glfwSetKeyCallback(*window, keyCallback);
}

void InputSystem::pollEvents() {
    std::fill(std::begin(mKeysPressedDown), std::end(mKeysPressedDown), false);
    std::fill(std::begin(mKeysReleasedUp), std::end(mKeysReleasedUp), false);

    mDeltaMouseX = 0;
    mDeltaMouseY = 0;

    glfwPollEvents();

    mQuitRequested = glfwWindowShouldClose(*mWindow.get());
}

void InputSystem::waitForEvents() {
    glfwWaitEvents();
}

void InputSystem::setCursorCaptured(bool cursorCaptured) {
    if (cursorCaptured) {
        glfwSetInputMode(*mWindow.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported()) {
            glfwSetInputMode(*mWindow.get(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        glfwGetCursorPos(*mWindow.get(), &mPreviousMouseX, &mPreviousMouseY);
        glfwSetCursorPosCallback(*mWindow.get(), cursorPositionCallback);
    } else {
        mDeltaMouseX = 0;
        mDeltaMouseY = 0;

        glfwSetCursorPosCallback(*mWindow.get(), nullptr);

        glfwSetInputMode(*mWindow.get(), GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        if (glfwRawMouseMotionSupported()) {
            glfwSetInputMode(*mWindow.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    mCursorCaptured = cursorCaptured;
}

void InputSystem::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    InputSystem& inputSystem = Engine::getInstance().getInputSystem();
    if (action == GLFW_PRESS) {
        inputSystem.mKeysHeldDown[key] = true;
        inputSystem.mKeysPressedDown[key] = true;
    } else if (action == GLFW_RELEASE) {
        inputSystem.mKeysHeldDown[key] = false;
        inputSystem.mKeysReleasedUp[key] = true;
    }
}

void InputSystem::cursorPositionCallback(GLFWwindow* window, double mouseX, double mouseY) {
    InputSystem& inputSystem = Engine::getInstance().getInputSystem();
    if (inputSystem.mCursorCaptured) {
        inputSystem.mDeltaMouseX += mouseX - inputSystem.mPreviousMouseX;
        inputSystem.mDeltaMouseY += mouseY - inputSystem.mPreviousMouseY;

        inputSystem.mPreviousMouseX = mouseX;
        inputSystem.mPreviousMouseY = mouseY;
    }
}
