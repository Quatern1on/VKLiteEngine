#include "pch.h"
#include "VulkanUtils.h"

vk::raii::ShaderModule VulkanUtils::createShaderModule(const VulkanContext& context,
        const AssetDescriptor& shaderAsset) {
    std::vector<uint8_t> codeBytes = shaderAsset.getBytes();

    assertm(codeBytes.size() % 4 == 0, "Shader code size have to be multiple of 4");

    auto* codePtr = reinterpret_cast<uint32_t*>(codeBytes.data());

    vk::ShaderModuleCreateInfo shaderModuleCreateInfo{
            {},
            codeBytes.size(),
            codePtr
    };
    return vk::raii::ShaderModule(context.getDevice(), shaderModuleCreateInfo);
}

vk::raii::Semaphore VulkanUtils::createSemaphore(const VulkanContext& context) {
    static vk::SemaphoreCreateInfo createInfo{};
    return vk::raii::Semaphore(context.getDevice(), createInfo);
}

vk::raii::Fence VulkanUtils::createFence(const VulkanContext& context, bool signaled) {
    static vk::FenceCreateInfo createInfoSignaled({vk::FenceCreateFlagBits::eSignaled}, nullptr);
    static vk::FenceCreateInfo createInfoNotSignaled{{}, nullptr};
    return vk::raii::Fence(context.getDevice(), signaled ? createInfoSignaled : createInfoNotSignaled);
}

uint32_t VulkanUtils::calculateNumberOfMipLevels(vk::Extent3D extent) {
    return std::floor(std::log2(std::max(std::max(extent.width, extent.height), extent.depth))) + 1U;
}

void VulkanUtils::beginDebugLabel(const vk::raii::CommandBuffer& cmd, const std::string& name, glm::vec3 color) {
    vk::DebugUtilsLabelEXT label(
            name.c_str(),
            {color.r, color.g, color.b, 1.0f}
    );

    cmd.beginDebugUtilsLabelEXT(label);
}

void VulkanUtils::endDebugLabel(const vk::raii::CommandBuffer& cmd) {
    cmd.endDebugUtilsLabelEXT();
}
