#pragma once
#include "pch.h"

class SingleCommandBuffer {
public:
    explicit SingleCommandBuffer(const VulkanContext& context);

    SingleCommandBuffer(const SingleCommandBuffer&) = delete;

    SingleCommandBuffer& operator=(const SingleCommandBuffer&) = delete;

    inline vk::raii::CommandBuffer& operator*() const {
        return *mCommandBuffer;
    }

    vk::raii::CommandBuffer& begin();

    void end();

private:
    std::reference_wrapper<const VulkanContext> mContext;

    std::unique_ptr<vk::raii::CommandPool> mCommandPool;
    std::unique_ptr<vk::raii::CommandBuffer> mCommandBuffer;
};
