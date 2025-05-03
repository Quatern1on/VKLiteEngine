#pragma once
#include "pch.h"

#include "Rendering/DeferredShading/DeferredShading.h"
#include "Vulkan/SingleCommandBuffer.h"
#include "Rendering/PostProcessing/Bloom.h"
#include "Rendering/PostProcessing/ToneMapping.h"

enum class BufferVisualization {
    eTonemapped,
    eMainColor,
    eAlbedo,
    eNormals,
    eMetallicRoughness,
    eBloom
};

class FrameRenderer {
public:
    FrameRenderer(const VulkanContext& context, SwapChain& swapChain, Scene& scene);

    FrameRenderer(const FrameRenderer&) = delete;

    FrameRenderer& operator=(const FrameRenderer&) = delete;

    void renderFrame();

    inline uint32_t getWidth() const {
        return mWidth;
    }

    inline uint32_t getHeight() const {
        return mHeight;
    }

    inline uint32_t getCurrentFrameImageIndex() const {
        return mCurrentFrameImageIndex;
    }

    inline vk::Format getMainColorImageFormat() const {
        return mMainColorImageFormat;
    }

    inline vk::Format getMainDepthImageFormat() const {
        return mMainDepthImageFormat;
    }

    inline const AllocatedImage& getMainColorImage() const {
        return *mMainColorImage;
    }

    inline const AllocatedImage& getMainDepthImage() const {
        return *mMainDepthImage;
    }

    inline const vk::raii::ImageView& getMainColorImageView() const {
        return *mMainColorImageView;
    }

    inline const vk::raii::ImageView& getMainDepthImageView() const {
        return *mMainDepthImageView;
    }

    inline BufferVisualization getCurrentBufferVisualization() const {
        return mBufferVisualization;
    }

    inline const Bloom& getBloom() const {
        return *mBloom;
    }

private:
    std::reference_wrapper<const VulkanContext> mContext;
    std::reference_wrapper<SwapChain> mSwapChain;
    std::reference_wrapper<Scene> mScene;

    BufferVisualization mBufferVisualization = BufferVisualization::eTonemapped;

    std::unique_ptr<DeferredShading> mDeferredShading;

    std::unique_ptr<Bloom> mBloom;

    std::unique_ptr<ToneMapping> mToneMapping;

    std::unique_ptr<vk::raii::Semaphore> mSwapChainImageReadySemaphore;
    std::unique_ptr<vk::raii::Semaphore> mRenderSemaphore;
    std::unique_ptr<vk::raii::Fence> mFrameFence;

    std::unique_ptr<SingleCommandBuffer> mCommandBuffer;

    uint32_t mCurrentFrameImageIndex = 0;

    uint32_t mWidth;
    uint32_t mHeight;

    vk::Format mMainColorImageFormat = vk::Format::eR16G16B16A16Sfloat;
    vk::Format mMainDepthImageFormat = vk::Format::eD32Sfloat;

    std::unique_ptr<AllocatedImage> mMainColorImage;
    std::unique_ptr<vk::raii::ImageView> mMainColorImageView;

    std::unique_ptr<AllocatedImage> mMainDepthImage;
    std::unique_ptr<vk::raii::ImageView> mMainDepthImageView;

private:
    void syncFrame();

    void createMainColorImage();

    void createMainDepthImage();

    void checkInput();

    void visualizeBuffer();

    void submitCommandBuffer();
};
