#include "pch.h"
#include "SpinComponent.h"

void SpinComponent::update(float delta) {
    static float time = 0.0f;
    time += delta;
    static glm::vec3 initialPosition = getSceneObject()->getTransform().getPosition();
    glm::vec3 newPosition = initialPosition;
    newPosition.y += std::sinf(time / 2.0) * 0.25;

    getSceneObject()->getTransform().rotate(.1f * delta, glm::vec3(0.0f, 1.0f, 0.0));
    getSceneObject()->getTransform().setPosition(newPosition);
}
