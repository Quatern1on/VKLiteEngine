#pragma once
#include "pch.h"

namespace VulkanUtils {
    vk::raii::ShaderModule createShaderModule(const VulkanContext &context,
                                              const AssetDescriptor &shaderAsset);

    vk::raii::Semaphore createSemaphore(const VulkanContext &context);

    vk::raii::Fence createFence(const VulkanContext &context, bool signaled);

    uint32_t calculateNumberOfMipLevels(vk::Extent3D extent);

    void beginDebugLabel(const vk::raii::CommandBuffer &cmd, const std::string &name, glm::vec3 color);

    void endDebugLabel(const vk::raii::CommandBuffer &cmd);
};

#if defined(VULKAN_DEBUG)
#define BEGIN_DEBUG_LABEL(cmd, name, color)         \
do {                                                \
    VulkanUtils::beginDebugLabel(cmd, name, color); \
} while (false)
#define END_DEBUG_LABEL(cmd)            \
do {                                    \
    VulkanUtils::endDebugLabel(cmd);    \
} while (false)
#else
#define BEGIN_DEBUG_LABEL(cmd, name, color) do {} while (false)
#define END_DEBUG_LABEL(cmd) do {} while (false)
#endif
