#pragma once
#include "pch.h"

class AssetFactory {
public:
    static std::unique_ptr<AssetDescriptor> getShaderAsset(const std::string& name);

    static std::unique_ptr<AssetDescriptor> getModelAsset(const std::string& name);

    static std::unique_ptr<AssetDescriptor> getTextureAsset(const std::string& name);

    static void init();

private:
    static std::string mShaderBasePath;
    static std::string mModelBasePath;
    static std::string mTextureBasePath;
};