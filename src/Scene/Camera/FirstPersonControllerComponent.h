#pragma once
#include "pch.h"

#include "Scene/BehaviourComponent.h"

class FirstPersonControllerComponent : public BehaviourComponent {
public:
    explicit FirstPersonControllerComponent();

    FirstPersonControllerComponent(const FirstPersonControllerComponent&) = delete;

    FirstPersonControllerComponent& operator=(const FirstPersonControllerComponent&) = delete;

    void update(float delta) override;

private:
    //By following this formula sensitivity will be equivalent to CS GO sensitivity
    constexpr static float kPixelToRadian = 2.0f * glm::pi<float>() / 16384.0f;

    float mSensitivity = 2.0f;

    float mSpeed = 3.0f;
    float mSprintSpeed = 10.0f;

    float mYaw = 0.0f;
    float mPitch = 0.0f;
};
