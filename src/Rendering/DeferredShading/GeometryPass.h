#pragma once
#include "pch.h"

#include "Scene/Mesh/MeshComponent.h"
#include "Vulkan/DescriptorGenerator.h"
#include "Vulkan/UniformBuffer.h"

class DeferredShading;

struct ObjectUniform {
    alignas(16) glm::mat4 modelMat;
    alignas(16) glm::mat3x4 normalMat;
    alignas(16) glm::mat4 modelViewProjectionMat;
};

struct MaterialUniform {
    alignas(16) glm::vec3 albedo;
    alignas(16) glm::vec3 emissive;
    alignas(4) float metallic;
    alignas(4) float roughness;
};

class GeometryPass {
public:
    GeometryPass(const VulkanContext& context, const DeferredShading& deferredShading);

    GeometryPass(const GeometryPass&) = delete;

    GeometryPass& operator=(const GeometryPass&) = delete;

    void render(vk::raii::CommandBuffer& commandBuffer,
            const std::vector<std::reference_wrapper<MeshComponent>>& meshes, const CameraComponent& camera);

private:
    std::reference_wrapper<const VulkanContext> mContext;
    std::reference_wrapper<const DeferredShading> mDeferredShading;

    std::unique_ptr<vk::raii::PipelineLayout> mPipelineLayout;
    std::unique_ptr<vk::raii::Pipeline> mPipeline;

    std::unique_ptr<DescriptorGenerator> mObjectDescriptorGenerator;
    std::unique_ptr<DescriptorGenerator> mMaterialDescriptorGenerator;

    std::unique_ptr<UniformBuffer> mUBO;

    std::unique_ptr<Texture2D> mDefaultOneTexture;
    std::unique_ptr<Texture2D> mDefaultOneSmallTexture;
    std::unique_ptr<Texture2D> mDefaultZeroTexture;
    std::unique_ptr<Texture2D> mDefaultNormalTexture;

private:
    void createDescriptorGenerators();

    void createPipeline();

    void createDefaultTextures();

    void writeMaterialDescriptors(const std::vector<std::reference_wrapper<MeshComponent>>& meshes,
            std::unordered_map<std::shared_ptr<Material>, uint64_t>& materialIndices,
            std::vector<vk::DescriptorSet>& materialDescriptors);

    void writeObjectDescriptors(const std::vector<std::reference_wrapper<MeshComponent>>& meshes,
            std::vector<vk::DescriptorSet>& objectDescriptors, const CameraComponent& camera);
};
