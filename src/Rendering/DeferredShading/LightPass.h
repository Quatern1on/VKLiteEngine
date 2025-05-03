#pragma once
#include "pch.h"

#include "Scene/Light/PointLightComponent.h"
#include "LightVolumeMesh.h"

class FrameRenderer;

struct LightVolumeUniform {
    alignas(16) glm::mat4 modelViewProjectionMatrix;
};

struct PointLightFragmentUniform {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 intensity;
    alignas(4) float radius;
};

struct FrameUniform {
    alignas(16) glm::vec3 cameraOrigin;
    alignas(16) glm::mat4 inverseTransform;
    alignas(8) glm::vec2 screenSize;
};

class LightPass {
public:
    LightPass(const VulkanContext& context, const DeferredShading& deferredShading, const FrameRenderer& frameRenderer);

    LightPass(const GeometryPass&) = delete;

    LightPass& operator=(const GeometryPass&) = delete;

    void render(vk::raii::CommandBuffer& commandBuffer, const Scene& scene);

private:
    std::reference_wrapper<const VulkanContext> mContext;
    std::reference_wrapper<const DeferredShading> mDeferredShading;

    std::unique_ptr<vk::raii::PipelineLayout> mPipelineLayout;
    std::unique_ptr<vk::raii::Pipeline> mPipeline;

    std::unique_ptr<DescriptorGenerator> mSubpassInputDescriptorGenerator;
    vk::DescriptorSet mSubpassInputDescriptorSet;

    std::unique_ptr<DescriptorGenerator> mFrameDescriptorGenerator;

    std::unique_ptr<DescriptorGenerator> mObjectDescriptorGenerator;

    std::unique_ptr<UniformBuffer> mUBO;

    std::unique_ptr<LightVolumeMesh> mPointLightVolumeMesh;

private:
    void createSubpassInputDescriptor(const FrameRenderer& frameRenderer);

    void createDescriptorGenerators();

    void createPipeline();

    vk::DescriptorSet writeFrameDescriptors(const CameraComponent& camera);

    void writeObjectDescriptors(const std::vector<std::reference_wrapper<PointLightComponent>>& pointLights,
            std::vector<vk::DescriptorSet>& objectDescriptors, const CameraComponent& camera);
};
