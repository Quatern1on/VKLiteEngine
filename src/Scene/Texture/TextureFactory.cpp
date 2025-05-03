#include "pch.h"
#include "TextureFactory.h"

#include <stb_image.h>

struct StbiDeleter {
    void operator()(uint8_t* data) { stbi_image_free(data); }
};

using stbi_ptr = std::unique_ptr<uint8_t[], StbiDeleter>;

TextureFactory::TextureFactory(const VulkanContext& context)
        : mContext(context) {}

std::unique_ptr<Texture2D> TextureFactory::load2D(std::unique_ptr<AssetDescriptor> textureAsset, bool linearSpace,
        int channels) {
    std::vector<uint8_t> bytes = textureAsset->getBytes();

    vk::Format format;
    if (linearSpace) {
        switch (channels) {
            case 1:
                format = vk::Format::eR8Unorm;
                break;
            case 2:
                format = vk::Format::eR8G8Unorm;
                break;
            case 3:
            case 4:
                channels = 4;
                format = vk::Format::eR8G8B8A8Unorm;
                break;
            default:
                throw std::runtime_error("Wrong number of channels, could be 1, 2, 3 or 4");
        }
    } else {
        switch (channels) {
            case 1:
                format = vk::Format::eR8Srgb;
                break;
            case 2:
                format = vk::Format::eR8G8Srgb;
                break;
            case 3:
            case 4:
                channels = 4;
                format = vk::Format::eR8G8B8A8Srgb;
                break;
            default:
                throw std::runtime_error("Wrong number of channels, could be 1, 2, 3 or 4");
        }
    }

    int width, height;
    stbi_ptr data = stbi_ptr(stbi_load_from_memory(bytes.data(), bytes.size(), &width, &height, nullptr, channels));

    uint64_t size = width * height * channels;

    return std::make_unique<Texture2D>(mContext, width, height, format, true, data.get(),
            size, textureAsset->getName());
}
