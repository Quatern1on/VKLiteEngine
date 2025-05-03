#include "pch.h"
#include "Allocator.h"

Allocator::Allocator(const vk::raii::Instance& instance, const vk::raii::PhysicalDevice& physicalDevice,
        const vk::raii::Device& device) {
    VmaAllocatorCreateInfo createInfo{
            {},
            *physicalDevice,
            *device,
            0,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            *instance,
            VK_API_VERSION_1_3,
            nullptr
    };

    vmaCreateAllocator(&createInfo, &mVmaAllocator);

    mMemoryProperties = physicalDevice.getMemoryProperties();
}

Allocator::~Allocator() {
    vmaDestroyAllocator(mVmaAllocator);
}

const VmaAllocator& Allocator::operator*() const {
    return mVmaAllocator;
}
