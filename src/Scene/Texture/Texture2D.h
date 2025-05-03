#pragma once
#include "pch.h"

class Texture2D {
public:
    Texture2D(const VulkanContext& context, uint32_t width, uint32_t height, vk::Format format, bool useMips,
            void* data, uint64_t size, const std::string& name = "");

    Texture2D(const Texture2D&) = delete;

    Texture2D& operator=(const Texture2D&) = delete;

    void transitionLayout(vk::ImageLayout newLayout);

    inline VkImage operator*() const {
        return **mImage;
    }

    inline const vk::raii::ImageView& getImageView() const {
        return *mImageView;
    }

    inline const vk::raii::Sampler& getSampler() const {
        return *mSampler;
    }

private:
    std::reference_wrapper<const VulkanContext> mContext;

    bool mMipsEnabled;

    uint32_t mMipLevels;

    uint32_t mWidth;
    uint32_t mHeight;

    vk::Format mFormat;

    std::unique_ptr<AllocatedImage> mImage;

    std::unique_ptr<vk::raii::ImageView> mImageView;

    std::unique_ptr<vk::raii::Sampler> mSampler;

private:
};
