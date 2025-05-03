#pragma once
#include "pch.h"
#include "Vulkan/Allocator/Allocator.h"
#include "Core/Window.h"

#if !defined(NDEBUG) && !defined(DISABLE_VALIDATION_LAYERS)
#define ENABLE_VALIDATION_LAYERS
#endif

class VulkanContext {
public:
    explicit VulkanContext(const Window& window);

    VulkanContext(const VulkanContext&) = delete;

    VulkanContext& operator=(const VulkanContext&) = delete;

    void executeCommandBufferSync(std::function<void(const vk::raii::CommandBuffer& cmd)>&& recordBuffer) const;

    inline const vk::raii::Context& getContext() const {
        return *mContext;
    }

    inline const vk::raii::Instance& getInstance() const {
        return *mInstance;
    }

    inline const vk::raii::SurfaceKHR& getSurface() const {
        return *mSurface;
    }

    inline const vk::raii::PhysicalDevice& getPhysicalDevice() const {
        return *mPhysicalDevice;
    }

    inline const vk::raii::Device& getDevice() const {
        return *mDevice;
    }

    inline const Allocator& getAllocator() const {
        return *mAllocator;
    }

    inline uint32_t getMainQueueFamilyIndex() const {
        return mMainQueueFamilyIndex;
    }

    inline const vk::raii::Queue& getMainQueue() const {
        return *mMainQueue;
    }

    inline const std::vector<std::string>& getEnabledLayers() const {
        return mEnabledLayers;
    }

    inline const std::vector<std::string>& getEnabledInstanceExtensions() const {
        return mEnabledInstanceExtensions;
    }

    inline const std::vector<std::string>& getEnabledDeviceExtensions() const {
        return mEnabledDeviceExtensions;
    }

    inline const vk::PhysicalDeviceFeatures& getEnabledDeviceFeatures() const {
        return mEnabledDeviceFeatures;
    }

    inline const vk::PhysicalDeviceProperties getPhysicalDeviceProperties() const {
        return mPhysicalDeviceProperties;
    }

private:
    std::unique_ptr<vk::raii::Context> mContext;
    std::unique_ptr<vk::raii::Instance> mInstance;
    std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> mDebugMessenger;
    std::unique_ptr<vk::raii::SurfaceKHR> mSurface;
    std::unique_ptr<vk::raii::PhysicalDevice> mPhysicalDevice;
    std::unique_ptr<vk::raii::Device> mDevice;
    std::unique_ptr<Allocator> mAllocator;

    std::unique_ptr<vk::raii::CommandPool> mSyncOperationsCommandPool;
    std::unique_ptr<vk::raii::CommandBuffer> mSyncOperationsCommandBuffer;
    std::unique_ptr<vk::raii::Fence> mSyncOperationsFence;

    uint32_t mMainQueueFamilyIndex;
    std::unique_ptr<vk::raii::Queue> mMainQueue;

    std::reference_wrapper<const Window> mWindow;

    std::vector<std::string> mEnabledLayers;
    std::vector<std::string> mEnabledInstanceExtensions;
    std::vector<std::string> mEnabledDeviceExtensions;
    vk::PhysicalDeviceFeatures mEnabledDeviceFeatures;
    vk::PhysicalDeviceProperties mPhysicalDeviceProperties;

private:
    void createInstance();

    void createDebugMessenger();

    void selectPhysicalDevice();

    void createDevice();

    void retrieveMainQueue();

    void createSyncOperationsObjects();

    std::vector<std::string> getRequiredLayers() const;

    std::vector<std::string> getRequiredInstanceExtensions() const;

    std::vector<std::string> getRequiredDeviceExtensions() const;

    vk::PhysicalDeviceFeatures getRequiredDeviceFeatures() const;

    bool checkLayerSupport(const std::vector<std::string>& layers);

    bool checkInstanceExtensionSupport(const std::vector<std::string>& extensions);

    bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& device,
            const std::vector<std::string>& extensions);

    bool checkDeviceFeaturesSupport(const vk::raii::PhysicalDevice& device, const vk::PhysicalDeviceFeatures& features);

    bool checkDeviceSupport(const vk::raii::PhysicalDevice& device);

    std::optional<uint32_t> selectMainQueue(const vk::raii::PhysicalDevice& device);
};
