#include "pch.h"
#include "FirstPersonControllerComponent.h"

FirstPersonControllerComponent::FirstPersonControllerComponent() {
    Engine::getInstance().getInputSystem().setCursorCaptured(true);
}

void FirstPersonControllerComponent::update(float delta) {
    TransformComponent& transform = getSceneObject()->getTransform();
    InputSystem& inputSystem = Engine::getInstance().getInputSystem();

    float mouseX = static_cast<float>(inputSystem.getMouseX() * kPixelToRadian);
    float mouseY = static_cast<float>(inputSystem.getMouseY() * kPixelToRadian);

    bool forwardPressed = inputSystem.getKey(Key::eW);
    bool backwardPressed = inputSystem.getKey(Key::eS);
    bool leftPressed = inputSystem.getKey(Key::eA);
    bool rightPressed = inputSystem.getKey(Key::eD);
    bool upPressed = inputSystem.getKey(Key::eE) || inputSystem.getKey(Key::eSpace);
    bool downPressed = inputSystem.getKey(Key::eQ);
    bool sprintPressed = inputSystem.getKey(Key::eLeftShift);

    glm::vec3 velocity(0.0f);

    glm::vec3 forward = glm::rotateY(glm::vec3(0.0f, 0.0f, 1.0), mYaw);
    glm::vec3 right = glm::rotateY(glm::vec3(1.0f, 0.0f, 0.0), mYaw);

    if (forwardPressed) {
        velocity += forward;
    }
    if (backwardPressed) {
        velocity -= forward;
    }
    if (rightPressed) {
        velocity += right;
    }
    if (leftPressed) {
        velocity -= right;
    }

    if (glm::length(velocity) > 0.1f) {
        velocity = glm::normalize(velocity);
    }

    if (upPressed) {
        velocity.y += 1.0f;
    }
    if (downPressed) {
        velocity.y -= 1.0f;
    }

    if (sprintPressed) {
        velocity *= mSprintSpeed;
    } else {
        velocity *= mSpeed;
    }

    transform.translate(velocity * delta);

    mPitch += mouseY * mSensitivity;
    mYaw += mouseX * mSensitivity;

    if (mPitch > glm::half_pi<float>()) {
        mPitch = glm::half_pi<float>();
    }
    if (mPitch < -glm::half_pi<float>()) {
        mPitch = -glm::half_pi<float>();
    }

    transform.setRotation(glm::quat(glm::vec3(mPitch, mYaw, 0.0f)));

    if (inputSystem.getKeyDown(Key::eTab)) {
        inputSystem.setCursorCaptured(!inputSystem.isCursorCaptured());
    }
}
