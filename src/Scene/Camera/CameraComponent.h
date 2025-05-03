#pragma once
#include "pch.h"

#include "Scene/Component.h"
#include "Scene/Camera/Camera.h"

class CameraComponent : public Component {
public:
    explicit CameraComponent(std::unique_ptr<Camera> camera);

    CameraComponent(const CameraComponent&) = delete;

    CameraComponent& operator=(const CameraComponent&) = delete;

    inline const Camera& getCamera() const {
        return *mCamera;
    }

    inline Camera& getCamera() {
        return *mCamera;
    }

    glm::mat4 getViewMatrix() const;

private:
    std::unique_ptr<Camera> mCamera;
};
