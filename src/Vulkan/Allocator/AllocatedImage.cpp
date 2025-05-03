#include "pch.h"
#include "AllocatedImage.h"

#include "Vulkan/Allocator/Allocator.h"
#include "StagingBuffer.h"

#include <vk_mem_alloc.h>

AllocatedImage::AllocatedImage(const VulkanContext& context, const vk::ImageCreateInfo& createInfo,
        const VmaAllocationCreateInfo& allocationCreateInfo)
        : mContext(std::ref(context)) {
    auto result = static_cast<vk::Result>(vmaCreateImage(*context.getAllocator(),
            reinterpret_cast<const VkImageCreateInfo*>(&createInfo),
            &allocationCreateInfo,
            &mNativeImage,
            &mAllocation,
            nullptr
    ));

    if (result != vk::Result::eSuccess) {
        vk::detail::throwResultException(result, "vmaCreateImage");
    }

    LOG(INFO) << "Vulkan image allocated. Extent = {" << createInfo.extent.width << ", "
              << createInfo.extent.height << ", " << createInfo.extent.depth << "}. Format = "
              << string_VkFormat(static_cast<VkFormat>(createInfo.format)) << ". Usage = "
              << string_VkBufferUsageFlags(static_cast<VkBufferUsageFlags>(createInfo.usage));
}

AllocatedImage::~AllocatedImage() {
    destroy();
}

void AllocatedImage::destroy() {
    vmaDestroyImage(*mContext.get().getAllocator(), mNativeImage, mAllocation);
}

VkImage AllocatedImage::operator*() const {
    return mNativeImage;
}

void AllocatedImage::uploadData(const void* data, uint64_t size, vk::ImageLayout imageLayout,
        const vk::ArrayProxy<const vk::BufferImageCopy>& regions) {
    LOG(INFO) << "Creating staging buffer for a transfer to image of " << size << " bytes";
    StagingBuffer stagingBuffer(mContext, size);
    stagingBuffer.transferToImage(data, size, mNativeImage, imageLayout, regions);
}