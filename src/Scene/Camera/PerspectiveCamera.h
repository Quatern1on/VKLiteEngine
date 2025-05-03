#pragma once
#include "pch.h"

#include "Scene/Camera/Camera.h"

class PerspectiveCamera : public Camera {
public:
    explicit PerspectiveCamera(uint32_t width, uint32_t height, float fovY, float near, float far);

    void update() override;

    inline float getFovY() const {
        return mFovY;
    }

    inline float getNear() const {
        return mNear;
    }

    inline float getFar() const {
        return mFar;
    }

    inline void setFovY(float fovY) {
        mFovY = fovY;
    }

    inline void setNear(float near) {
        mNear = near;
    }

    inline void setFar(float far) {
        mFar = far;
    }

private:
    float mFovY;
    float mNear;
    float mFar;
};
