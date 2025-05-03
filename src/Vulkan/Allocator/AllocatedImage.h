#pragma once
#include "pch.h"

#include "Vulkan/Allocator/Allocator.h"

#include <vk_mem_alloc.h>

class AllocatedImage {
public:
    explicit AllocatedImage(const VulkanContext& context, const vk::ImageCreateInfo& createInfo,
            const VmaAllocationCreateInfo& allocationCreateInfo = {0, VMA_MEMORY_USAGE_AUTO});

    AllocatedImage(const AllocatedImage&) = delete;

    AllocatedImage& operator=(const AllocatedImage&) = delete;

    ~AllocatedImage();

    VkImage operator*() const;

    void uploadData(const void* data, uint64_t size, vk::ImageLayout imageLayout,
            const vk::ArrayProxy<const vk::BufferImageCopy>& regions);

private:
    std::reference_wrapper<const VulkanContext> mContext;
    VmaAllocation mAllocation{};

    VkImage mNativeImage{};

private:
    void destroy();
};
