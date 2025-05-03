#include "pch.h"
#include "StagingBuffer.h"

StagingBuffer::StagingBuffer(const VulkanContext& context, uint64_t size)
        : mContext(std::ref(context)), mSize(size) {
    uint32_t queueFamilyIndex = context.getMainQueueFamilyIndex();

    vk::BufferCreateInfo createInfo(
            {},
            size,
            {vk::BufferUsageFlagBits::eTransferSrc},
            vk::SharingMode::eExclusive,
            1, &queueFamilyIndex
    );

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VmaAllocationInfo allocationInfo;

    auto result = static_cast<vk::Result>(vmaCreateBuffer(*context.getAllocator(),
            reinterpret_cast<const VkBufferCreateInfo*>(&createInfo),
            &allocationCreateInfo,
            &mNativeBuffer,
            &mAllocation,
            &allocationInfo
    ));

    if (result != vk::Result::eSuccess) {
        vk::detail::throwResultException(result, "vmaCreateBuffer");
    }

    const vk::MemoryType& allocatedMemoryType = context.getAllocator().getMemoryProperties()
            .memoryTypes[allocationInfo.memoryType];
    mHostCoherent = (allocatedMemoryType.propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent)
                    == vk::MemoryPropertyFlagBits::eHostCoherent;
}

StagingBuffer::~StagingBuffer() {
    clear();
}

void StagingBuffer::clear() {
    vmaDestroyBuffer(*mContext.get().getAllocator(), mNativeBuffer, mAllocation);
}

void StagingBuffer::transferToBuffer(const void* data, vk::Buffer dstBuffer, uint64_t dstOffset, uint64_t size) const {
    if (size > mSize) {
        throw std::runtime_error("The size of a transfer operation is greater than the size of a staging "
                                 "buffer");
    }

    void* mappedPointer;

    vmaMapMemory(*mContext.get().getAllocator(), mAllocation, &mappedPointer);
    std::memcpy(mappedPointer, data, size);
    if (!mHostCoherent) {
        vmaFlushAllocation(*mContext.get().getAllocator(), mAllocation, 0, size);
    }
    vmaUnmapMemory(*mContext.get().getAllocator(), mAllocation);

    mContext.get().executeCommandBufferSync([=, this](const vk::raii::CommandBuffer& cmd) {
        cmd.copyBuffer(mNativeBuffer, dstBuffer, {{0, dstOffset, size}});
    });
}

void StagingBuffer::transferToImage(const void* data, uint64_t size, vk::Image dstImage, vk::ImageLayout imageLayout,
        const vk::ArrayProxy<const vk::BufferImageCopy>& regions) const {
    void* mappedPointer;

    vmaMapMemory(*mContext.get().getAllocator(), mAllocation, &mappedPointer);
    std::memcpy(mappedPointer, data, size);
    if (!mHostCoherent) {
        vmaFlushAllocation(*mContext.get().getAllocator(), mAllocation, 0, size);
    }
    vmaUnmapMemory(*mContext.get().getAllocator(), mAllocation);

    mContext.get().executeCommandBufferSync([=, this](const vk::raii::CommandBuffer& cmd) {
        cmd.copyBufferToImage(mNativeBuffer, dstImage, imageLayout, regions);
    });
}
