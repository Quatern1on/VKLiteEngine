#pragma once
#include "pch.h"

#include "Scene/Component.h"

class PointLightComponent : public Component {
public:
    explicit PointLightComponent(glm::vec3 intensity);

    PointLightComponent(const PointLightComponent&) = delete;

    PointLightComponent& operator=(const PointLightComponent&) = delete;

    void setIntensity(glm::vec3 intensity);

    inline glm::vec3 getIntensity() const {
        return mIntensity;
    }

    inline float getRadius() const {
        return mRadius;
    }

private:
    glm::vec3 mIntensity;

    float mRadius;
};
