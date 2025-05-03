#include "pch.h"
#include "PerspectiveCamera.h"

PerspectiveCamera::PerspectiveCamera(uint32_t width, uint32_t height, float fovY, float near, float far)
        : Camera(width, height), mFovY(fovY), mNear(near), mFar(far) {}

void PerspectiveCamera::update() {
    mProjectionMatrix = glm::perspective(mFovY, static_cast<float>(getWidth()) / static_cast<float>(getHeight()),
            mNear, mFar);
    mProjectionMatrix[1][1] *= -1.0f;

    mInverseProjectionMatrix = glm::inverse(mProjectionMatrix);
}
