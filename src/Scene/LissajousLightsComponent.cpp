#include "pch.h"
#include <Scene/Mesh/MeshLoader.h>
#include "LissajousLightsComponent.h"

LissajousLightsComponent::LissajousLightsComponent(uint32_t numberOfLights)
        : mNumberOfLights(numberOfLights) {}

void LissajousLightsComponent::update(float delta) {
    static float t = 0.0f;
    t += delta * 0.1f;

    for (uint32_t i = 0; i < mNumberOfLights; i++) {
        float param = t + static_cast<float>(i) * static_cast<float>(2.0 * std::numbers::pi)
                          / static_cast<float>(mNumberOfLights);
        float x = 8.0f * std::sin(param);
        float y = 8.0f * sin(7.0f * param + 1.3);
        float z = 8.0f * sin(12.0f * param + 4.2);

        mLights[i].get().setPosition(glm::vec3(x, y, z));
    }
}

glm::vec3 HSVtoRGB(float H, float S, float V) {
    if (H > 360 || H < 0 || S > 100 || S < 0 || V > 100 || V < 0) {
        throw std::runtime_error("Wrong HSV");
    }

    float s = S / 100;
    float v = V / 100;
    float C = s * v;
    float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
    float m = v - C;
    float r, g, b;
    if (H >= 0 && H < 60) {
        r = C, g = X, b = 0;
    } else if (H >= 60 && H < 120) {
        r = X, g = C, b = 0;
    } else if (H >= 120 && H < 180) {
        r = 0, g = C, b = X;
    } else if (H >= 180 && H < 240) {
        r = 0, g = X, b = C;
    } else if (H >= 240 && H < 300) {
        r = X, g = 0, b = C;
    } else {
        r = C, g = 0, b = X;
    }
    int R = (r + m) * 255;
    int G = (g + m) * 255;
    int B = (b + m) * 255;

    return glm::vec3(static_cast<float>(R) / 255.0f, static_cast<float>(G) / 255.0f, static_cast<float>(B) / 255.0f);
}

void LissajousLightsComponent::attachToSceneObject(SceneObject* sceneObject) {
    Component::attachToSceneObject(sceneObject);

    mLights.reserve(mNumberOfLights);

    std::shared_ptr<Mesh> mesh = MeshLoader::loadAsSingleMesh(AssetFactory::getModelAsset("point_light_volume.obj"));

    for (uint32_t i = 0; i < mNumberOfLights; i++) {
        float H = static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
        H = static_cast<float>(i) / mNumberOfLights;
        glm::vec3 color = HSVtoRGB(H * 360.0f, 100.0f, 100.0f);

        std::unique_ptr<MeshComponent> meshComponent = std::make_unique<MeshComponent>(mesh);
        meshComponent->getMaterialSlots()[0] = std::make_shared<Material>(glm::vec3(0.0f),
                color * 200.0f, 0.0f, 0.0f);

        std::unique_ptr<SceneObject> lightObject = std::make_unique<SceneObject>();
        lightObject->addComponent(std::make_unique<PointLightComponent>(color * 32.0f));
        lightObject->addComponent(std::move(meshComponent));
        lightObject->getTransform().setScale(glm::vec3(0.1f, 0.1f, 0.1f));
        mLights.push_back(std::ref(lightObject->getTransform()));
        sceneObject->addChild(std::move(lightObject));
    }
}
