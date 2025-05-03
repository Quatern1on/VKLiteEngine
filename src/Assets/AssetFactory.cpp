#include "pch.h"
#include "AssetFactory.h"

#include <filesystem>

std::string AssetFactory::mShaderBasePath;
std::string AssetFactory::mModelBasePath;
std::string AssetFactory::mTextureBasePath;

void AssetFactory::init() {
    if (std::filesystem::exists("../assets")) {
        //Debug filesystem layout
        AssetFactory::mShaderBasePath = "shaders/";
        AssetFactory::mModelBasePath = "../assets/models/";
        AssetFactory::mTextureBasePath = "../assets/textures/";
    } else if (std::filesystem::exists("./assets")) {
        //Distribution filesystem layout
        AssetFactory::mShaderBasePath = "assets/shaders/";
        AssetFactory::mModelBasePath = "assets/models/";
        AssetFactory::mTextureBasePath = "assets/textures/";
    } else {
        throw std::runtime_error("Assets folder not found");
    }
}

std::unique_ptr<AssetDescriptor> AssetFactory::getShaderAsset(const std::string& name) {
    return std::make_unique<FileAssetDescriptor>(mShaderBasePath + name + ".spv", name);
}

std::unique_ptr<AssetDescriptor> AssetFactory::getModelAsset(const std::string& name) {
    return std::make_unique<FileAssetDescriptor>(mModelBasePath + name, name);
}

std::unique_ptr<AssetDescriptor> AssetFactory::getTextureAsset(const std::string& name) {
    return std::make_unique<FileAssetDescriptor>(mTextureBasePath + name, name);
}
