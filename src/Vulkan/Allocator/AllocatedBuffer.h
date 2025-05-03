#pragma once
#include "pch.h"
#include "Vulkan/Allocator/Allocator.h"

#include <vk_mem_alloc.h>

class AllocatedBuffer {
public:
    explicit AllocatedBuffer(const VulkanContext& context, const vk::BufferCreateInfo& createInfo,
            const VmaAllocationCreateInfo& allocationCreateInfo = {0, VMA_MEMORY_USAGE_AUTO});

    AllocatedBuffer(const AllocatedBuffer&) = delete;

    AllocatedBuffer& operator=(const AllocatedBuffer&) = delete;

    ~AllocatedBuffer();

    VkBuffer operator*() const;

    void uploadData(const void* data, uint64_t offset, uint64_t size);

    void* mapMemory();

    void unmapMemory();

    void flushMemory(uint64_t offset, uint64_t size);

    void clearAndResize(uint64_t newSize);

    inline bool isHostVisible() const {
        return mHostVisible;
    }

    inline bool isHostCoherent() const {
        return mHostCoherent;
    }

private:
    std::reference_wrapper<const VulkanContext> mContext;
    vk::BufferCreateInfo mCreateInfo;
    VmaAllocationCreateInfo mAllocationCreateInfo;

    VmaAllocation mAllocation{};

    VkBuffer mNativeBuffer{};

    bool mHostVisible;
    bool mHostCoherent;

private:
    void allocateNativeBuffer();

    void destroy();
};
