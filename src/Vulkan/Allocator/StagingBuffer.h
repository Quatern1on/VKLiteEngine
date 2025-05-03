#pragma once
#include "pch.h"

#include "Vulkan/Allocator/Allocator.h"

#include <vk_mem_alloc.h>

class StagingBuffer {
public:
    explicit StagingBuffer(const VulkanContext& context, uint64_t size);

    StagingBuffer(const StagingBuffer&) = delete;

    StagingBuffer& operator=(const StagingBuffer&) = delete;

    ~StagingBuffer();

    void transferToBuffer(const void* data, vk::Buffer dstBuffer, uint64_t dstOffset, uint64_t size) const;

    void transferToImage(const void* data, uint64_t size, vk::Image dstImage, vk::ImageLayout imageLayout,
            const vk::ArrayProxy<const vk::BufferImageCopy>& regions) const;

    inline uint64_t getSize() const {
        return mSize;
    }

private:
    std::reference_wrapper<const VulkanContext> mContext;
    VmaAllocation mAllocation{};

    VkBuffer mNativeBuffer{};

    bool mHostCoherent;

    uint64_t mSize;

    void clear();
};
