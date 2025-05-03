#pragma once
#include "pch.h"

#include "Rendering/DeferredShading/GBuffer.h"
#include "Rendering/DeferredShading/GeometryPass.h"
#include "Vulkan/UniformBuffer.h"
#include "LightPass.h"

class FrameRenderer;

class DeferredShading {
public:
    DeferredShading(const VulkanContext& context, const FrameRenderer& frameRenderer, uint32_t width, uint32_t height);

    DeferredShading(const DeferredShading&) = delete;

    DeferredShading& operator=(const DeferredShading&) = delete;

    void render(Scene& scene, vk::raii::CommandBuffer& commandBuffer);

    inline GBuffer& getGBuffer() {
        return *mGBuffer;
    }

    inline const GBuffer& getGBuffer() const {
        return *mGBuffer;
    }

    inline const vk::raii::RenderPass& getRenderPass() const {
        return *mRenderPass;
    }

private:
    const std::vector<GBufferImageDescription> kGBufferImageDescriptions = {
            {"albedo",             vk::Format::eR8G8B8A8Srgb},              //4 bytes
            {"normal_ao",          vk::Format::eR16G16B16A16Unorm},         //8 bytes
            {"metallic_roughness", vk::Format::eR8G8Unorm},                 //2 bytes
    };

    std::reference_wrapper<const VulkanContext> mContext;
    std::reference_wrapper<const FrameRenderer> mFrameRenderer;

    std::unique_ptr<GBuffer> mGBuffer;

    std::unique_ptr<vk::raii::RenderPass> mRenderPass;
    std::unique_ptr<vk::raii::Framebuffer> mFrameBuffer;

    std::vector<vk::ClearValue> mClearValues;

    std::unique_ptr<GeometryPass> mGeometryPass;
    std::unique_ptr<LightPass> mLightPass;

private:
    void createRenderPass();

    void createFramebuffer();

    void createClearValues();
};
