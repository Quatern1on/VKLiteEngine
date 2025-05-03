#include "pch.h"
#include "Texture2D.h"

Texture2D::Texture2D(const VulkanContext& context, uint32_t width, uint32_t height, vk::Format format, bool useMips,
        void* data, uint64_t size, const std::string& name)
        : mContext(std::ref(context)), mWidth(width), mHeight(height), mFormat(format), mMipsEnabled(useMips) {
    vk::Extent3D extent(
            width,
            height,
            1
    );

    uint32_t queueFamilyIndex = context.getMainQueueFamilyIndex();

    mMipLevels = (useMips) ? VulkanUtils::calculateNumberOfMipLevels(extent) : 1;

    vk::ImageCreateInfo imageCreateInfo(
            {},
            vk::ImageType::e2D,
            format,
            extent,
            mMipLevels,
            1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            {vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
             vk::ImageUsageFlagBits::eTransferSrc},
            vk::SharingMode::eExclusive,
            1, &queueFamilyIndex,
            vk::ImageLayout::eUndefined
    );

    mImage = std::make_unique<AllocatedImage>(context, imageCreateInfo);
#ifdef VULKAN_DEBUG
    context.getDevice().setDebugUtilsObjectNameEXT({
            vk::ObjectType::eImage,
            reinterpret_cast<uint64_t>(**mImage),
            (name + "_Texture2D_Image").c_str()
    });
#endif

    vk::ImageSubresourceLayers subresource(
            {vk::ImageAspectFlagBits::eColor},
            0,
            0,
            1
    );

    vk::BufferImageCopy region(
            0,
            0,
            0,
            subresource,
            {0, 0, 0},
            extent
    );

    transitionLayout(vk::ImageLayout::eTransferDstOptimal);

    mImage->uploadData(data, size, vk::ImageLayout::eTransferDstOptimal, {region});

    context.executeCommandBufferSync([=, this](const vk::raii::CommandBuffer& cmd) {
        int32_t dstWidth = width;
        int32_t dstHeight = height;
        for (uint32_t mipLevel = 1; mipLevel < mMipLevels; mipLevel++) {
            int32_t srcWidth = dstWidth;
            int32_t srcHeight = dstHeight;
            dstWidth = std::max(srcWidth / 2, 1);
            dstHeight = std::max(srcHeight / 2, 1);

            vk::ImageMemoryBarrier transitionSrc(
                    vk::AccessFlagBits::eTransferWrite,
                    vk::AccessFlagBits::eTransferRead,
                    vk::ImageLayout::eTransferDstOptimal,
                    vk::ImageLayout::eTransferSrcOptimal,
                    queueFamilyIndex,
                    queueFamilyIndex,
                    **mImage,
                    {
                            vk::ImageAspectFlagBits::eColor,
                            mipLevel - 1,
                            1,
                            0,
                            1
                    }
            );
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
                    {}, {}, {}, {transitionSrc});

            vk::ImageBlit blit(
                    {
                            vk::ImageAspectFlagBits::eColor,
                            mipLevel - 1,
                            0,
                            1
                    },
                    {
                            vk::Offset3D{0, 0, 0},
                            vk::Offset3D{srcWidth, srcHeight, 1}
                    },
                    {
                            vk::ImageAspectFlagBits::eColor,
                            mipLevel,
                            0,
                            1
                    },
                    {
                            vk::Offset3D{0, 0, 0},
                            vk::Offset3D{dstWidth, dstHeight, 1}
                    }
            );
            cmd.blitImage(**mImage, vk::ImageLayout::eTransferSrcOptimal,
                    **mImage, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);
        }

        vk::ImageMemoryBarrier transitionAll(
                vk::AccessFlagBits::eNone,
                vk::AccessFlagBits::eNone,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eShaderReadOnlyOptimal,
                queueFamilyIndex,
                queueFamilyIndex,
                **mImage,
                {
                        vk::ImageAspectFlagBits::eColor,
                        0,
                        mMipLevels,
                        0,
                        1
                }
        );
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe,
                {}, {}, {}, {transitionAll});
    });

//    transitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    vk::ImageViewCreateInfo imageViewCreateInfo(
            {},
            vk::Image(**mImage),
            vk::ImageViewType::e2D,
            format,
            {},
            {
                    {vk::ImageAspectFlagBits::eColor},
                    0,
                    mMipLevels,
                    0,
                    1
            }
    );
    mImageView = std::make_unique<vk::raii::ImageView>(context.getDevice(), imageViewCreateInfo);

    vk::SamplerCreateInfo samplerCreateInfo(
            {},
            vk::Filter::eLinear,
            vk::Filter::eLinear,
            vk::SamplerMipmapMode::eLinear,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            0.0f,
            true,
            16,
            false,
            vk::CompareOp::eNever,
            0.0f,
            static_cast<float>(mMipLevels),
            vk::BorderColor::eIntOpaqueBlack,
            false
    );
    mSampler = std::make_unique<vk::raii::Sampler>(context.getDevice(), samplerCreateInfo);
}

void Texture2D::transitionLayout(vk::ImageLayout newLayout) {
    mContext.get().executeCommandBufferSync([=, this](const vk::raii::CommandBuffer& cmd) {
        vk::ImageMemoryBarrier imageMemoryBarrier(
                vk::AccessFlagBits::eMemoryWrite,
                vk::AccessFlagBits::eMemoryRead,
                vk::ImageLayout::eUndefined,
                newLayout,
                mContext.get().getMainQueueFamilyIndex(),
                mContext.get().getMainQueueFamilyIndex(),
                **mImage,
                {
                        vk::ImageAspectFlagBits::eColor,
                        0,
                        mMipLevels,
                        0,
                        1
                }
        );

        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eBottomOfPipe, vk::PipelineStageFlagBits::eTopOfPipe,
                {}, {}, {}, {imageMemoryBarrier});
    });
}
