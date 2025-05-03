#include "pch.h"
#include "GBuffer.h"

GBuffer::GBuffer(const VulkanContext& context, std::vector<GBufferImageDescription> imageDescriptions,
        uint32_t width, uint32_t height)
        : mContext(std::ref(context)), mImageDescriptions(std::move(imageDescriptions)),
          mWidth(width), mHeight(height) {
    createImages();
    createImageViews();
}

void GBuffer::createImages() {
    mImages.clear();
    mImages.reserve(mImageDescriptions.size());

    for (auto& imageDescription : mImageDescriptions) {
        vk::Extent3D imageExtent(
                mWidth,
                mHeight,
                1
        );

        uint32_t mainQueueFamilyIndex = mContext.get().getMainQueueFamilyIndex();

        vk::ImageCreateInfo imageCreateInfo(
                {},
                vk::ImageType::e2D,
                imageDescription.format,
                imageExtent,
                1,
                1,
                vk::SampleCountFlagBits::e1,
                vk::ImageTiling::eOptimal,
                {vk::ImageUsageFlagBits::eColorAttachment
                 | vk::ImageUsageFlagBits::eSampled
                 | vk::ImageUsageFlagBits::eInputAttachment
                 //TODO remove this flag
                 | vk::ImageUsageFlagBits::eTransferSrc},
                vk::SharingMode::eExclusive,
                mainQueueFamilyIndex,
                vk::ImageLayout::eUndefined
        );

        mImages.emplace_back(std::make_unique<AllocatedImage>(mContext, imageCreateInfo));

#if defined(VULKAN_DEBUG)
        mContext.get().getDevice().setDebugUtilsObjectNameEXT({
                vk::ObjectType::eImage,
                reinterpret_cast<uint64_t>(**mImages.back()),
                (imageDescription.name + "_GBuffer_Image").c_str()
        });
#endif
    }
}

void GBuffer::createImageViews() {
    mImageViews.clear();
    mImageViews.reserve(mImageDescriptions.size());

    vk::ImageSubresourceRange imageSubresourceRange(
            {vk::ImageAspectFlagBits::eColor},
            0, 1,
            0, 1
    );

    for (int i = 0; i < mImageDescriptions.size(); i++) {
        vk::ImageViewCreateInfo imageViewCreateInfo(
                {},
                **mImages[i],
                vk::ImageViewType::e2D,
                mImageDescriptions[i].format,
                {},
                imageSubresourceRange
        );

        mImageViews.emplace_back(std::make_unique<vk::raii::ImageView>
                (mContext.get().getDevice(), imageViewCreateInfo));

#if defined(VULKAN_DEBUG)
        mContext.get().getDevice().setDebugUtilsObjectNameEXT({
                vk::ObjectType::eImageView,
                reinterpret_cast<uint64_t>(static_cast<VkImageView>(**mImageViews.back())),
                (mImageDescriptions[i].name + "_GBuffer_ImageView").c_str()
        });
#endif
    }
}
