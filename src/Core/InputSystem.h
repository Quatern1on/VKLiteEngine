#pragma once
#include "pch.h"

#include "KeyCodes.h"

class InputSystem {
public:
    explicit InputSystem(const Window& window);

    void pollEvents();

    void waitForEvents();

    void setCursorCaptured(bool cursorCaptured);

    inline bool isQuitRequested() const {
        return mQuitRequested;
    }

    inline bool isCursorCaptured() const {
        return mCursorCaptured;
    }

    inline bool getKey(Key key) const {
        return mKeysHeldDown[static_cast<uint16_t>(key)];
    }

    inline bool getKeyDown(Key key) const {
        return mKeysPressedDown[static_cast<uint16_t>(key)];
    }

    inline bool getKeyUp(Key key) const {
        return mKeysReleasedUp[static_cast<uint16_t>(key)];
    }

    inline double getMouseX() const {
        return mDeltaMouseX;
    }

    inline double getMouseY() const {
        return mDeltaMouseY;
    }

private:
    std::reference_wrapper<const Window> mWindow;

    bool mQuitRequested = false;

    bool mCursorCaptured = false;

    bool mKeysHeldDown[static_cast<uint16_t>(Key::eMax) + 1]{};
    bool mKeysPressedDown[static_cast<uint16_t>(Key::eMax) + 1]{};
    bool mKeysReleasedUp[static_cast<uint16_t>(Key::eMax) + 1]{};

    double mDeltaMouseX = 0;
    double mDeltaMouseY = 0;

    double mPreviousMouseX = 0;
    double mPreviousMouseY = 0;

private:
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void cursorPositionCallback(GLFWwindow* window, double mouseX, double mouseY);
};
