#include "pch.h"
#include "PointLightComponent.h"

PointLightComponent::PointLightComponent(glm::vec3 intensity) {
    setIntensity(intensity);
}

void PointLightComponent::setIntensity(glm::vec3 intensity) {
    mIntensity = intensity;
    float maxIntensity = std::max(std::max(intensity.x, intensity.y), intensity.z);
    mRadius = 8.0f * std::sqrt(maxIntensity);
}
