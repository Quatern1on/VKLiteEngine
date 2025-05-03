#include "pch.h"
#include "SingleCommandBuffer.h"

SingleCommandBuffer::SingleCommandBuffer(const VulkanContext& context)
        : mContext(std::ref(context)) {
    vk::CommandPoolCreateInfo commandPoolCreateInfo(
            {vk::CommandPoolCreateFlagBits::eTransient},
            mContext.get().getMainQueueFamilyIndex()
    );

    mCommandPool = std::make_unique<vk::raii::CommandPool>(mContext.get().getDevice(), commandPoolCreateInfo);

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo(
            **mCommandPool,
            vk::CommandBufferLevel::ePrimary,
            1
    );

    mCommandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(mContext.get().getDevice()
            .allocateCommandBuffers(commandBufferAllocateInfo)[0]));
}

vk::raii::CommandBuffer& SingleCommandBuffer::begin() {
    mCommandPool->reset();
    mCommandBuffer->begin({{vk::CommandBufferUsageFlagBits::eOneTimeSubmit}, nullptr});
    return *mCommandBuffer;
}

void SingleCommandBuffer::end() {
    mCommandBuffer->end();
}