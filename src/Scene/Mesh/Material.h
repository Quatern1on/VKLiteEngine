#pragma once
#include "pch.h"

#include "Scene/Texture/Texture2D.h"

struct Material {
public:
    static const std::shared_ptr<Material> kDefaultMaterial;

    explicit Material() = default;

    explicit Material(glm::vec3 albedo, glm::vec3 emissive, float metallic, float roughness);

public:
    glm::vec3 albedo;
    std::shared_ptr<Texture2D> albedoTexture;

    glm::vec3 emissive;
    std::shared_ptr<Texture2D> emissiveTexture;

    float metallic;
    std::shared_ptr<Texture2D> metallicTexture;

    float roughness;
    std::shared_ptr<Texture2D> roughnessTexture;

    std::shared_ptr<Texture2D> normalTexture;

    std::shared_ptr<Texture2D> aoTexture;
};
