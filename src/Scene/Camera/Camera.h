#pragma once
#include "pch.h"

#include "Scene/Component.h"

class Camera {
public:
    explicit Camera(uint32_t width, uint32_t height);

    virtual void update() = 0;

    inline glm::mat4 getProjectionMatrix() const {
        return mProjectionMatrix;
    };

    virtual glm::mat4 getInverseProjectionMatrix() const {
        return mInverseProjectionMatrix;
    };

    inline uint32_t getWidth() const {
        return mWidth;
    }

    inline uint32_t getHeight() const {
        return mHeight;
    }

    inline void setWidth(uint32_t width) {
        mWidth = width;
    }

    inline void setHeight(uint32_t height) {
        mHeight = height;
    }

protected:
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);
    glm::mat4 mInverseProjectionMatrix = glm::mat4(1.0f);

private:
    uint32_t mWidth;
    uint32_t mHeight;
};
