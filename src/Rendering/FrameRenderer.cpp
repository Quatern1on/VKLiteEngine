#include "pch.h"
#include "FrameRenderer.h"

#include "Vulkan/VulkanUtils.h"

FrameRenderer::FrameRenderer(const VulkanContext& context, SwapChain& swapChain, Scene& scene)
        : mContext(std::ref(context)), mSwapChain(std::ref(swapChain)), mScene(std::ref(scene)) {
    mWidth = swapChain.getWidth();
    mHeight = swapChain.getHeight();

    mSwapChainImageReadySemaphore = std::make_unique<vk::raii::Semaphore>(VulkanUtils::createSemaphore(context));
    mRenderSemaphore = std::make_unique<vk::raii::Semaphore>(VulkanUtils::createSemaphore(context));
    mFrameFence = std::make_unique<vk::raii::Fence>(VulkanUtils::createFence(context, true));

    createMainColorImage();
    createMainDepthImage();

    mDeferredShading = std::make_unique<DeferredShading>(context, *this, mWidth, mHeight);

    mBloom = std::make_unique<Bloom>(context, *this, mWidth, mHeight);
    mToneMapping = std::make_unique<ToneMapping>(context, *this);

    mCommandBuffer = std::make_unique<SingleCommandBuffer>(context);
}

void FrameRenderer::renderFrame() {
    syncFrame();
    checkInput();

    try {
        mCurrentFrameImageIndex = mSwapChain.get().acquireNextImage(**mSwapChainImageReadySemaphore);
    } catch (vk::OutOfDateKHRError& error) {
        return;
    }

    mScene.get().getCamera()->getCamera().update();

    vk::raii::CommandBuffer& commandBuffer = mCommandBuffer->begin();
    BEGIN_DEBUG_LABEL(commandBuffer, "Main Frame Render", glm::vec3(0.0f, 0.0f, 0.0f));

    mDeferredShading->render(mScene, commandBuffer);

    BEGIN_DEBUG_LABEL(commandBuffer, "Post Processing", glm::vec3(0.0f, 0.0f, 1.0f));

    mBloom->render(commandBuffer);
    mToneMapping->render(commandBuffer);

    END_DEBUG_LABEL(commandBuffer);

    visualizeBuffer();

    END_DEBUG_LABEL(commandBuffer);
    mCommandBuffer->end();

    submitCommandBuffer();

    try {
        mSwapChain.get().present(mCurrentFrameImageIndex, **mRenderSemaphore);
    } catch (vk::OutOfDateKHRError& error) {}
}

void FrameRenderer::syncFrame() {
    using namespace std::chrono_literals;
    std::chrono::nanoseconds timeout = 10s;

    vk::Result result = mContext.get().getDevice()
            .waitForFences({**mFrameFence}, true, timeout.count());
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Error while waiting for frame fence");
    }
    mContext.get().getDevice().resetFences({**mFrameFence});
}

void FrameRenderer::createMainColorImage() {
    uint32_t mainQueueFamilyIndex = mContext.get().getMainQueueFamilyIndex();

    vk::Extent3D imageExtent(
            mWidth,
            mHeight,
            1
    );

    vk::ImageCreateInfo imageCreateInfo(
            {},
            vk::ImageType::e2D,
            mMainColorImageFormat,
            imageExtent,
            1,
            1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            {vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
             | vk::ImageUsageFlagBits::eTransferSrc},
            vk::SharingMode::eExclusive,
            mainQueueFamilyIndex,
            vk::ImageLayout::eUndefined
    );
    mMainColorImage = std::make_unique<AllocatedImage>(mContext, imageCreateInfo);

    vk::ImageSubresourceRange imageSubresourceRange(
            {vk::ImageAspectFlagBits::eColor},
            0, 1,
            0, 1
    );

    vk::ImageViewCreateInfo imageViewCreateInfo(
            {},
            **mMainColorImage,
            vk::ImageViewType::e2D,
            mMainColorImageFormat,
            {},
            imageSubresourceRange
    );
    mMainColorImageView = std::make_unique<vk::raii::ImageView>(mContext.get().getDevice(), imageViewCreateInfo);

#if defined(VULKAN_DEBUG)
    mContext.get().getDevice().setDebugUtilsObjectNameEXT({
            vk::ObjectType::eImage,
            reinterpret_cast<uint64_t>(**mMainColorImage),
            "main_color_Image"
    });
    mContext.get().getDevice().setDebugUtilsObjectNameEXT({
            vk::ObjectType::eImageView,
            reinterpret_cast<uint64_t>(static_cast<VkImageView>(**mMainColorImageView)),
            "main_color_ImageView"
    });
#endif
}

void FrameRenderer::createMainDepthImage() {
    uint32_t mainQueueFamilyIndex = mContext.get().getMainQueueFamilyIndex();

    vk::Extent3D imageExtent(
            mWidth,
            mHeight,
            1
    );

    vk::ImageCreateInfo imageCreateInfo(
            {},
            vk::ImageType::e2D,
            mMainDepthImageFormat,
            imageExtent,
            1,
            1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            {vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled
             | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eInputAttachment},
            vk::SharingMode::eExclusive,
            mainQueueFamilyIndex,
            vk::ImageLayout::eUndefined
    );
    mMainDepthImage = std::make_unique<AllocatedImage>(mContext, imageCreateInfo);

    vk::ImageSubresourceRange imageSubresourceRange(
            {vk::ImageAspectFlagBits::eDepth},
            0, 1,
            0, 1
    );

    vk::ImageViewCreateInfo imageViewCreateInfo(
            {},
            **mMainDepthImage,
            vk::ImageViewType::e2D,
            mMainDepthImageFormat,
            {},
            imageSubresourceRange
    );
    mMainDepthImageView = std::make_unique<vk::raii::ImageView>(mContext.get().getDevice(), imageViewCreateInfo);

#if defined(VULKAN_DEBUG)
    mContext.get().getDevice().setDebugUtilsObjectNameEXT({
            vk::ObjectType::eImage,
            reinterpret_cast<uint64_t>(**mMainDepthImage),
            "main_depth_Image"
    });
    mContext.get().getDevice().setDebugUtilsObjectNameEXT({
            vk::ObjectType::eImageView,
            reinterpret_cast<uint64_t>(static_cast<VkImageView>(**mMainDepthImageView)),
            "main_depth_ImageView"
    });
#endif
}

void FrameRenderer::checkInput() {
    InputSystem& inputSystem = Engine::getInstance().getInputSystem();

    if (inputSystem.getKeyDown(Key::eN1)) {
        mBufferVisualization = BufferVisualization::eTonemapped;
    }
    if (inputSystem.getKeyDown(Key::eN2)) {
        mBufferVisualization = BufferVisualization::eMainColor;
    }
    if (inputSystem.getKeyDown(Key::eN3)) {
        mBufferVisualization = BufferVisualization::eAlbedo;
    }
    if (inputSystem.getKeyDown(Key::eN4)) {
        mBufferVisualization = BufferVisualization::eNormals;
    }
    if (inputSystem.getKeyDown(Key::eN5)) {
        mBufferVisualization = BufferVisualization::eMetallicRoughness;
    }
    if (inputSystem.getKeyDown(Key::eN6)) {
        mBufferVisualization = BufferVisualization::eBloom;
    }
}

void FrameRenderer::visualizeBuffer() {
    BEGIN_DEBUG_LABEL(**mCommandBuffer, "Buffer Visualization", glm::vec3(0.0f, 0.0f, 1.0f));

    vk::ImageSubresourceLayers imageSubresourceLayers(
            {vk::ImageAspectFlagBits::eColor},
            0,
            0,
            1
    );
    std::array<vk::Offset3D, 2> offsets = {vk::Offset3D(0, 0, 0),
                                           vk::Offset3D(mWidth, mHeight, 1)};
    vk::ImageBlit blit(
            imageSubresourceLayers,
            offsets,
            imageSubresourceLayers,
            offsets
    );

    vk::Image srcImage;

    switch (mBufferVisualization) {
        case BufferVisualization::eTonemapped:
            srcImage = *mToneMapping->getImage();
            break;
        case BufferVisualization::eMainColor:
            srcImage = **mMainColorImage;
            break;
        case BufferVisualization::eAlbedo:
            srcImage = *mDeferredShading->getGBuffer().getImage(0);
            break;
        case BufferVisualization::eNormals:
            srcImage = *mDeferredShading->getGBuffer().getImage(1);
            break;
        case BufferVisualization::eMetallicRoughness:
            srcImage = *mDeferredShading->getGBuffer().getImage(2);
            break;
        case BufferVisualization::eBloom:
            srcImage = *mBloom->getImage();
            break;
    }

    vk::ImageMemoryBarrier srcImageMemoryBarrier(
            {vk::AccessFlagBits::eColorAttachmentWrite},
            {vk::AccessFlagBits::eTransferRead},
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferSrcOptimal,
            mContext.get().getMainQueueFamilyIndex(),
            mContext.get().getMainQueueFamilyIndex(),
            srcImage,
            {
                    {vk::ImageAspectFlagBits::eColor},
                    0, 1,
                    0, 1
            }
    );
    (**mCommandBuffer).pipelineBarrier(
            {vk::PipelineStageFlagBits::eColorAttachmentOutput},
            {vk::PipelineStageFlagBits::eTransfer},
            {},
            {},
            {},
            {srcImageMemoryBarrier}
    );

    vk::ImageMemoryBarrier swapchainImageMemoryBarrier(
            {vk::AccessFlagBits::eNone},
            {vk::AccessFlagBits::eTransferWrite},
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferDstOptimal,
            mContext.get().getMainQueueFamilyIndex(),
            mContext.get().getMainQueueFamilyIndex(),
            Engine::getInstance().getSwapChain().getImage(
                    Engine::getInstance().getFrameRenderer().getCurrentFrameImageIndex()),
            {
                    {vk::ImageAspectFlagBits::eColor},
                    0, 1,
                    0, 1
            }
    );
    (**mCommandBuffer).pipelineBarrier(
            {vk::PipelineStageFlagBits::eTopOfPipe},
            {vk::PipelineStageFlagBits::eTransfer},
            {},
            {},
            {},
            {swapchainImageMemoryBarrier}
    );

    (**mCommandBuffer).blitImage(
            srcImage,
            vk::ImageLayout::eTransferSrcOptimal,
            Engine::getInstance().getSwapChain().getImage(
                    Engine::getInstance().getFrameRenderer().getCurrentFrameImageIndex()),
            vk::ImageLayout::eTransferDstOptimal,
            blit,
            vk::Filter::eLinear
    );

    vk::ImageMemoryBarrier swapchainImageMemoryBarrier2(
            {vk::AccessFlagBits::eTransferWrite},
            {vk::AccessFlagBits::eNone},
            vk::ImageLayout::eTransferDstOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            mContext.get().getMainQueueFamilyIndex(),
            mContext.get().getMainQueueFamilyIndex(),
            Engine::getInstance().getSwapChain().getImage(
                    Engine::getInstance().getFrameRenderer().getCurrentFrameImageIndex()),
            {
                    {vk::ImageAspectFlagBits::eColor},
                    0, 1,
                    0, 1
            }
    );
    (**mCommandBuffer).pipelineBarrier(
            {vk::PipelineStageFlagBits::eTransfer},
            {vk::PipelineStageFlagBits::eBottomOfPipe},
            {},
            {},
            {},
            {swapchainImageMemoryBarrier2}
    );

    END_DEBUG_LABEL(**mCommandBuffer);
}

void FrameRenderer::submitCommandBuffer() {
    static vk::PipelineStageFlags waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submitInfo(
            1, &**mSwapChainImageReadySemaphore,
            &waitStages,
            1, &***mCommandBuffer,
            1, &**mRenderSemaphore
    );
    mContext.get().getMainQueue().submit({submitInfo}, **mFrameFence);
}
