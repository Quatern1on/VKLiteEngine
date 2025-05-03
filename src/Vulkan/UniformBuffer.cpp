#include "pch.h"
#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(const VulkanContext& context, uint64_t size)
        : mContext(std::ref(context)), mSize(size), mCurrentLocation(0) {
    mAlignment = context.getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
    createBufferObject();

    mMappedPointer = mBuffer->mapMemory();
}

UniformBuffer::~UniformBuffer() {
    mBuffer->unmapMemory();
}

void UniformBuffer::createBufferObject() {
    vk::BufferCreateInfo bufferCreateInfo(
            {},
            mSize,
            {vk::BufferUsageFlagBits::eUniformBuffer},
            vk::SharingMode::eExclusive,
            {}
    );

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    mBuffer = std::make_unique<AllocatedBuffer>(mContext.get(), bufferCreateInfo, allocationCreateInfo);

    assertm(mBuffer->isHostVisible(), "Uniform buffer must be host visible");
}

VkBuffer UniformBuffer::operator*() const {
    return **mBuffer;
}

uint64_t UniformBuffer::pushData(const void* data, uint64_t size) {
    assertm(mCurrentLocation + size <= mSize, "Insufficient uniform buffer size");

    std::memcpy(static_cast<uint8_t*>(mMappedPointer) + mCurrentLocation, data, size);
    if (!mBuffer->isHostCoherent()) {
        mBuffer->flushMemory(mCurrentLocation, size);
    }

    uint64_t offset = mCurrentLocation;

    mCurrentLocation += (size + mAlignment - 1) & ~(mAlignment - 1);

    return offset;
}

void UniformBuffer::restart() {
    mCurrentLocation = 0;
}
