#pragma once
#include "pch.h"

#include "Vulkan/Allocator/AllocatedImage.h"

struct GBufferImageDescription {
    std::string name;
    vk::Format format;
};

class GBuffer {
public:
    explicit GBuffer(const VulkanContext& context, std::vector<GBufferImageDescription> imageDescriptions,
            uint32_t width, uint32_t height);

    GBuffer(const GBuffer&) = delete;

    GBuffer& operator=(const GBuffer&) = delete;

    inline const std::vector<std::unique_ptr<AllocatedImage>>& getImages() const {
        return mImages;
    }

    inline const std::vector<std::unique_ptr<vk::raii::ImageView>>& getImageViews() const {
        return mImageViews;
    }

    inline const AllocatedImage& getImage(std::size_t index) const {
        return *mImages.at(index);
    }

    inline const vk::raii::ImageView& getImageView(std::size_t index) const {
        return *mImageViews.at(index);
    }

    inline uint32_t getWidth() const {
        return mWidth;
    }

    inline uint32_t getHeight() const {
        return mHeight;
    }

    inline std::size_t size() const {
        return mImages.size();
    }

    inline const std::vector<GBufferImageDescription>& getImageDescriptions() const {
        return mImageDescriptions;
    }

private:
    std::reference_wrapper<const VulkanContext> mContext;

    uint32_t mWidth;
    uint32_t mHeight;

    std::vector<GBufferImageDescription> mImageDescriptions;

    std::vector<std::unique_ptr<AllocatedImage>> mImages;
    std::vector<std::unique_ptr<vk::raii::ImageView>> mImageViews;

    void createImages();

    void createImageViews();
};
