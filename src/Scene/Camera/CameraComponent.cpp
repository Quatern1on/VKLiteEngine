#include "pch.h"
#include "CameraComponent.h"

CameraComponent::CameraComponent(std::unique_ptr<Camera> camera)
        : mCamera(std::move(camera)) {}

glm::mat4 CameraComponent::getViewMatrix() const {
    const TransformComponent& transform = getSceneObject()->getTransform();

    return glm::lookAt(transform.getPosition(), transform.getPosition() + transform.forward(),
            transform.up());
}
