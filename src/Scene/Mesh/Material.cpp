#include "pch.h"
#include "Material.h"

const std::shared_ptr<Material> Material::kDefaultMaterial = std::make_shared<Material>(
        glm::vec3(0.7f, 0.7f, 0.7f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.0f,
        0.5f
);

Material::Material(glm::vec3 albedo, glm::vec3 emissive, float metallic, float roughness)
        : albedo(albedo), emissive(emissive), metallic(metallic), roughness(roughness) {}
