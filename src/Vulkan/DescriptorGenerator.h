#pragma once
#include "pch.h"


class DescriptorGenerator {
public:
    DescriptorGenerator(const VulkanContext& context, const std::vector<vk::DescriptorSetLayoutBinding>& bindings,
            uint32_t initialPoolSize = 1000);

    DescriptorGenerator(const DescriptorGenerator&) = delete;

    DescriptorGenerator& operator=(const DescriptorGenerator&) = delete;

    /**
     * Resets the descriptor pool object recycling all descriptor sets previously allocated.
     * If new poolSize is greater than the current one, recreates the descriptor pool object with greater size.
     * @param poolSize Maximum amount of descriptor sets that could be allocated before next restart call
     */
    void restart(uint32_t poolSize);

    vk::DescriptorSet createDescriptorSet();

    std::vector<vk::DescriptorSet> createDescriptorSets(uint32_t amount);

    inline uint32_t getPoolSize() const {
        return mPoolSize;
    }

    inline vk::raii::DescriptorSetLayout& getLayout() {
        return *mDescriptorSetLayout;
    }

private:
    uint32_t mPoolSize;
    uint32_t mAvailable;

    std::reference_wrapper<const VulkanContext> mContext;

    std::vector<vk::DescriptorSetLayoutBinding> mBindings;

    std::unique_ptr<vk::raii::DescriptorPool> mDescriptorPool;

    std::unique_ptr<vk::raii::DescriptorSetLayout> mDescriptorSetLayout;

private:
    void createLayout();

    void createPool();
};
