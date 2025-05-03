#pragma once
#include "pch.h"

#include "BehaviourComponent.h"

class LissajousLightsComponent : public BehaviourComponent {
public:
    explicit LissajousLightsComponent(uint32_t numberOfLights);

    LissajousLightsComponent(const LissajousLightsComponent&) = delete;

    LissajousLightsComponent& operator=(const LissajousLightsComponent&) = delete;

    void update(float delta) override;

    void attachToSceneObject(SceneObject* sceneObject) override;

private:
    uint32_t mNumberOfLights;

    std::vector<std::reference_wrapper<TransformComponent>> mLights;
};
