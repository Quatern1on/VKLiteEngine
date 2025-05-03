#pragma once
#include "pch.h"

#include "Vulkan/Allocator/AllocatedBuffer.h"

class UniformBuffer {
public:
    explicit UniformBuffer(const VulkanContext& context, uint64_t size);

    UniformBuffer(const UniformBuffer&) = delete;

    UniformBuffer& operator=(const UniformBuffer&) = delete;

    ~UniformBuffer();

    VkBuffer operator*() const;

    uint64_t pushData(const void* data, uint64_t size);

    void restart();

private:
    std::reference_wrapper<const VulkanContext> mContext;

    std::unique_ptr<AllocatedBuffer> mBuffer;

    uint64_t mSize;

    uint64_t mCurrentLocation;

    uint32_t mAlignment;

    void* mMappedPointer = nullptr;

private:
    void createBufferObject();
};
