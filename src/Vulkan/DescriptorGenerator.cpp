#include "pch.h"
#include "DescriptorGenerator.h"

DescriptorGenerator::DescriptorGenerator(const VulkanContext& context,
        const std::vector<vk::DescriptorSetLayoutBinding>& bindings, uint32_t initialPoolSize)
        : mPoolSize(initialPoolSize), mContext(std::ref(context)), mBindings(bindings) {
    createLayout();

    createPool();
    mAvailable = initialPoolSize;
}

void DescriptorGenerator::restart(uint32_t poolSize) {
    if (poolSize > mPoolSize) {
        mPoolSize = std::max(poolSize, mPoolSize + mPoolSize / 2);
        createPool();
    } else {
        mDescriptorPool->reset({});
    }
    mAvailable = mPoolSize;
}

std::vector<vk::DescriptorSet> DescriptorGenerator::createDescriptorSets(uint32_t amount) {
    if (amount > mAvailable) {
        throw std::runtime_error("Attempt to create more descriptor sets from a descriptor generator than available");
    }

    std::vector<vk::DescriptorSetLayout> setLayouts(amount, **mDescriptorSetLayout);

    vk::DescriptorSetAllocateInfo allocateInfo(
            **mDescriptorPool,
            setLayouts
    );

    mAvailable -= amount;

    return (*mContext.get().getDevice()).allocateDescriptorSets(allocateInfo);
}

vk::DescriptorSet DescriptorGenerator::createDescriptorSet() {
    return createDescriptorSets(1)[0];
}

void DescriptorGenerator::createLayout() {
    vk::DescriptorSetLayoutCreateInfo createInfo(
            {},
            mBindings
    );

    mDescriptorSetLayout = std::make_unique<vk::raii::DescriptorSetLayout>(mContext.get().getDevice(), createInfo);
}

void DescriptorGenerator::createPool() {
    std::vector<vk::DescriptorPoolSize> sizes;
    sizes.reserve(mBindings.size());
    for (const auto& binding : mBindings) {
        sizes.emplace_back(binding.descriptorType, binding.descriptorCount * mPoolSize);
    }

    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
            {},
            mPoolSize,
            sizes
    );
    mDescriptorPool = std::make_unique<vk::raii::DescriptorPool>(mContext.get().getDevice(), descriptorPoolCreateInfo);
}
