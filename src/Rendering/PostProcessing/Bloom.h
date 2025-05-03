#pragma once
#include "pch.h"

class FrameRenderer;

class Bloom {
public:
    Bloom(const VulkanContext& context, const FrameRenderer& frameRenderer, uint32_t width, uint32_t height);

    Bloom(const Bloom&) = delete;

    Bloom& operator=(const Bloom&) = delete;

    inline vk::Format getFormat() const {
        return vk::Format::eB10G11R11UfloatPack32;
    }

    inline uint32_t getWidth() const {
        return mWidth;
    }

    inline uint32_t getHeight() const {
        return mHeight;
    }

    inline const AllocatedImage& getImage() const {
        return *mImages[0];
    }

    inline const vk::raii::ImageView& getImageView() const {
        return *mImageViews[0];
    }

    void render(vk::raii::CommandBuffer& commandBuffer);

private:
    std::reference_wrapper<const VulkanContext> mContext;
    std::reference_wrapper<const vk::raii::ImageView> mMainColorImageView;
    std::reference_wrapper<const FrameRenderer> mFrameRenderer;

    uint32_t mImagesCount = 0;

    uint32_t mWidth;
    uint32_t mHeight;

    std::vector<std::unique_ptr<AllocatedImage>> mImages;
    std::vector<std::unique_ptr<vk::raii::ImageView>> mImageViews;
    std::vector<vk::Extent3D> mExtents;

    std::unique_ptr<vk::raii::RenderPass> mRenderPass;

    std::vector<std::unique_ptr<vk::raii::Framebuffer>> mFrameBuffers;

    std::unique_ptr<vk::raii::Pipeline> mDownsamplePipeline;
    std::unique_ptr<vk::raii::Pipeline> mUpsamplePipeline;
    std::unique_ptr<vk::raii::PipelineLayout> mPipelineLayout;

    std::unique_ptr<vk::raii::Sampler> mLinearSampler;
    std::unique_ptr<vk::raii::Sampler> mNearestSampler;
    std::unique_ptr<DescriptorGenerator> mTextureDescriptorGenerator;
    std::vector<vk::DescriptorSet> mLinearDescriptors;
    std::vector<vk::DescriptorSet> mNearestDescriptors;

private:
    void createImages();

    void createRenderPasses();

    void createFramebuffers();

    void createTextureDescriptors();

    void createPipelines();
};
