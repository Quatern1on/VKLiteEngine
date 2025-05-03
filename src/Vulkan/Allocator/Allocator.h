#pragma once
#include "pch.h"

#include <vk_mem_alloc.h>

class Allocator {
public:
    explicit Allocator(const vk::raii::Instance& instance, const vk::raii::PhysicalDevice& physicalDevice,
            const vk::raii::Device& device);

    ~Allocator();

    Allocator(const Allocator&) = delete;

    Allocator& operator=(const Allocator&) = delete;

    const VmaAllocator& operator*() const;

    inline const vk::PhysicalDeviceMemoryProperties& getMemoryProperties() const {
        return mMemoryProperties;
    }

private:
    VmaAllocator mVmaAllocator{};

    vk::PhysicalDeviceMemoryProperties mMemoryProperties;
};
