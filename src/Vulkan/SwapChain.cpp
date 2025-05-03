#include "pch.h"
#include "SwapChain.h"

#include "Core/Engine.h"

SwapChain::SwapChain(const VulkanContext& context)
        : mFormat(vk::Format::eB8G8R8A8Srgb), mColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear), mExtent(0, 0),
          mContext(std::ref(context)) {
    createNativeSwapchain();
    retrieveImages();
    createImageViews();
}

uint32_t SwapChain::acquireNextImage(const vk::Semaphore& signalSemaphore, const vk::Fence& signalFence) {
    using namespace std::chrono_literals;
    std::chrono::nanoseconds timeout = 10s;

    try {
        auto [result, imageIndex] = mNativeSwapchain->acquireNextImage(timeout.count(),
                signalSemaphore, signalFence);

        if (result != vk::Result::eSuccess) {
            LOG(WARNING) << "Attempt to acquire next image from swapchain finished with result = "
                         << string_VkResult(static_cast<VkResult>(result));
        }

        return imageIndex;
    } catch (vk::OutOfDateKHRError& error) {
        LOG(INFO) << "Swapchain was out of date while trying to acquire next image";
        mOutOfDate = true;
        throw error;
    }
}

void SwapChain::present(uint32_t imageIndex, const vk::Semaphore& waitSemaphore) {
    vk::PresentInfoKHR presentInfo(
            (waitSemaphore) ? 1 : 0, &waitSemaphore,
            1, &**mNativeSwapchain, &imageIndex,
            nullptr
    );

    try {
        vk::Result result = mContext.get().getMainQueue().presentKHR(presentInfo);

        if (result != vk::Result::eSuccess) {
            LOG(WARNING) << "Attempt to present swapchain image finished with result = "
                         << string_VkResult(static_cast<VkResult>(result));
        }
    } catch (vk::OutOfDateKHRError& error) {
        LOG(INFO) << "Swapchain was out of date while trying to present image";
        mOutOfDate = true;
        throw error;
    }
}

void SwapChain::createNativeSwapchain() {
    auto surface = vk::SurfaceKHR(*mContext.get().getSurface());

    const std::vector<vk::SurfaceFormatKHR> surfaceFormats = mContext.get().getPhysicalDevice()
            .getSurfaceFormatsKHR(surface);

    const vk::SurfaceCapabilitiesKHR surfaceCapabilities = mContext.get().getPhysicalDevice()
            .getSurfaceCapabilitiesKHR(surface);

    vk::SurfaceFormatKHR surfaceFormat = selectFormat(surfaceFormats);
    vk::Extent2D surfaceExtent = selectExtent(surfaceCapabilities);

    vk::SwapchainCreateInfoKHR swapchainCreateInfo(
            {},
            surface,
            surfaceCapabilities.minImageCount,
            surfaceFormat.format,
            surfaceFormat.colorSpace,
            surfaceExtent,
            1,
            {vk::ImageUsageFlagBits::eColorAttachment |
             //TODO remove this
             vk::ImageUsageFlagBits::eTransferDst
            },
            vk::SharingMode::eExclusive,
            0, nullptr,
            surfaceCapabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            vk::PresentModeKHR::eMailbox,
            true,
            {}
    );

    mNativeSwapchain = std::make_unique<vk::raii::SwapchainKHR>(mContext.get().getDevice(), swapchainCreateInfo);

    mFormat = surfaceFormat.format;
    mColorSpace = surfaceFormat.colorSpace;
    mExtent = surfaceExtent;

    LOG(INFO) << "Swapchain created. Width = " << getWidth() << ", height = " << getHeight()
              << ", format = " << string_VkFormat(static_cast<VkFormat>(getFormat()));
}

void SwapChain::retrieveImages() {
    mImages = mNativeSwapchain->getImages();
}

void SwapChain::createImageViews() {
    mImageViews.clear();
    mImageViews.reserve(mImages.size());

    vk::ImageViewCreateInfo imageViewCreateInfo(
            {},
            {},
            vk::ImageViewType::e2D,
            mFormat,
            {
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
                    vk::ComponentSwizzle::eIdentity,
            },
            {
                    {vk::ImageAspectFlagBits::eColor},
                    0, 1,
                    0, 1,
            }
    );

    for (const auto& image : mImages) {
        imageViewCreateInfo.setImage(image);

        mImageViews.emplace_back(std::make_unique<vk::raii::ImageView>(
                mContext.get().getDevice().createImageView(imageViewCreateInfo)));
    }
}

vk::SurfaceFormatKHR SwapChain::selectFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    vk::SurfaceFormatKHR requiredFormat(
            vk::Format::eB8G8R8A8Srgb,
            vk::ColorSpaceKHR::eSrgbNonlinear
    );

    if (std::find(availableFormats.begin(), availableFormats.end(), requiredFormat) == std::end(availableFormats)) {
        throw std::runtime_error("Surface does not support required format");
    }

    return requiredFormat;
}

vk::Extent2D SwapChain::selectExtent(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities) {
    if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return surfaceCapabilities.currentExtent;
    } else {
        auto [width, height] = Engine::getInstance().getWindow().getSize();

        VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, surfaceCapabilities.minImageExtent.width,
                surfaceCapabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, surfaceCapabilities.minImageExtent.height,
                surfaceCapabilities.maxImageExtent.height);

        return actualExtent;
    }
}
