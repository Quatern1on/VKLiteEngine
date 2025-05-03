#include "pch.h"
#include "AllocatedBuffer.h"

#include "Vulkan/Allocator/Allocator.h"
#include "StagingBuffer.h"

#include <vk_mem_alloc.h>

AllocatedBuffer::AllocatedBuffer(const VulkanContext& context, const vk::BufferCreateInfo& createInfo,
        const VmaAllocationCreateInfo& allocationCreateInfo)
        : mContext(std::ref(context)), mCreateInfo(createInfo), mAllocationCreateInfo(allocationCreateInfo),
          mHostVisible(false), mHostCoherent(false) {
    allocateNativeBuffer();
}

AllocatedBuffer::~AllocatedBuffer() {
    destroy();
}

void AllocatedBuffer::allocateNativeBuffer() {
    VmaAllocationInfo allocationInfo;

    auto result = static_cast<vk::Result>(vmaCreateBuffer(*mContext.get().getAllocator(),
            reinterpret_cast<const VkBufferCreateInfo*>(&mCreateInfo),
            &mAllocationCreateInfo,
            &mNativeBuffer,
            &mAllocation,
            &allocationInfo
    ));

    if (result != vk::Result::eSuccess) {
        vk::detail::throwResultException(result, "vmaCreateBuffer");
    }

    const vk::MemoryType& allocatedMemoryType = mContext.get().getAllocator().getMemoryProperties()
            .memoryTypes[allocationInfo.memoryType];
    mHostVisible = (allocatedMemoryType.propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible)
                   == vk::MemoryPropertyFlagBits::eHostVisible;
    mHostCoherent = (allocatedMemoryType.propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent)
                    == vk::MemoryPropertyFlagBits::eHostCoherent;

    LOG(INFO) << "Vulkan buffer allocated. Size = " << mCreateInfo.size << " bytes, usage = "
              << string_VkBufferUsageFlags(static_cast<VkBufferUsageFlags>(mCreateInfo.usage));
}

void AllocatedBuffer::destroy() {
    vmaDestroyBuffer(*mContext.get().getAllocator(), mNativeBuffer, mAllocation);
}

VkBuffer AllocatedBuffer::operator*() const {
    return mNativeBuffer;
}

void AllocatedBuffer::uploadData(const void* data, uint64_t offset, uint64_t size) {
    if (mHostVisible) {
        void* mappedPointer;
        vmaMapMemory(*mContext.get().getAllocator(), mAllocation, &mappedPointer);
        std::memcpy(static_cast<uint8_t*>(mappedPointer) + offset, data, size);
        if (!mHostCoherent) {
            vmaFlushAllocation(*mContext.get().getAllocator(), mAllocation, 0, size);
        }
        vmaUnmapMemory(*mContext.get().getAllocator(), mAllocation);
    } else {
        LOG(INFO) << "Creating staging buffer for a transfer of " << size << " bytes";
        StagingBuffer stagingBuffer(mContext, size);
        stagingBuffer.transferToBuffer(data, vk::Buffer(mNativeBuffer), offset, size);
    }
}

void AllocatedBuffer::clearAndResize(uint64_t newSize) {
    destroy();

    mCreateInfo.size = newSize;

    allocateNativeBuffer();
}

void* AllocatedBuffer::mapMemory() {
    void* mappedPointer;
    vmaMapMemory(*mContext.get().getAllocator(), mAllocation, &mappedPointer);
    return mappedPointer;
}

void AllocatedBuffer::unmapMemory() {
    vmaUnmapMemory(*mContext.get().getAllocator(), mAllocation);
}

void AllocatedBuffer::flushMemory(uint64_t offset, uint64_t size) {
    vmaFlushAllocation(*mContext.get().getAllocator(), mAllocation, offset, size);
}


