#pragma once
#include "pch.h"

class SwapChain {
public:
    SwapChain(const VulkanContext& context);

    SwapChain(const SwapChain&) = delete;

    SwapChain& operator=(const SwapChain&) = delete;

    uint32_t acquireNextImage(const vk::Semaphore& signalSemaphore = {}, const vk::Fence& signalFence = {});

    void present(uint32_t imageIndex, const vk::Semaphore& waitSemaphore = {});

    inline vk::Format getFormat() const {
        return mFormat;
    }

    inline vk::ColorSpaceKHR getColorSpace() const {
        return mColorSpace;
    }

    inline vk::Extent2D getExtent() const {
        return mExtent;
    }

    inline uint32_t getWidth() const {
        return mExtent.width;
    }

    inline uint32_t getHeight() const {
        return mExtent.height;
    }

    inline const vk::Image& getImage(uint32_t imageIndex) const {
        return mImages[imageIndex];
    }

    inline const vk::raii::ImageView& getImageView(uint32_t imageIndex) const {
        return *mImageViews[imageIndex];
    }

    inline bool isOutOfDate() const {
        return mOutOfDate;
    }

private:
    std::reference_wrapper<const VulkanContext> mContext;

    std::unique_ptr<vk::raii::SwapchainKHR> mNativeSwapchain;

    std::vector<vk::Image> mImages;

    std::vector<std::unique_ptr<vk::raii::ImageView>> mImageViews;

    vk::Format mFormat;
    vk::ColorSpaceKHR mColorSpace;
    vk::Extent2D mExtent;

    bool mOutOfDate = false;

private:
    void createNativeSwapchain();

    void retrieveImages();

    void createImageViews();

    vk::SurfaceFormatKHR selectFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

    vk::Extent2D selectExtent(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities);
};
