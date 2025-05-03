#pragma once
#include "pch.h"

#include "Texture2D.h"

class TextureFactory {
public:
    explicit TextureFactory(const VulkanContext& context);

    TextureFactory(const TextureFactory&) = delete;

    TextureFactory& operator=(const TextureFactory&) = delete;

    std::unique_ptr<Texture2D> load2D(std::unique_ptr<AssetDescriptor> textureAsset, bool linearSpace, int channels);

private:
    std::reference_wrapper<const VulkanContext> mContext;
};
