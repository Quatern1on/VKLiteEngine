#pragma once
#include "pch.h"

class ToneMapping {
public:
    ToneMapping(const VulkanContext& context, const FrameRenderer& frameRenderer);

    ToneMapping(const ToneMapping&) = delete;

    ToneMapping& operator=(const ToneMapping&) = delete;

    inline const AllocatedImage& getImage() const {
        return *mImage;
    }

    inline const vk::raii::ImageView& getImageView() const {
        return *mImageView;
    }

    void render(vk::raii::CommandBuffer& commandBuffer);

private:
    std::reference_wrapper<const VulkanContext> mContext;
    std::reference_wrapper<const FrameRenderer> mFrameRenderer;

    std::unique_ptr<AllocatedImage> mImage;
    std::unique_ptr<vk::raii::ImageView> mImageView;

    std::unique_ptr<vk::raii::RenderPass> mRenderPass;

    std::unique_ptr<vk::raii::Framebuffer> mFrameBuffer;

    std::unique_ptr<vk::raii::Pipeline> mPipeline;
    std::unique_ptr<vk::raii::PipelineLayout> mPipelineLayout;

    std::unique_ptr<vk::raii::Sampler> mTextureSampler;
    std::unique_ptr<DescriptorGenerator> mTexturesDescriptorGenerator;
    vk::DescriptorSet mTexturesSet;

private:
    void createImage();

    void createRenderPass();

    void createFramebuffer();

    void createTextureDescriptor();

    void createPipeline();
};
