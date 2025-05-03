#include "pch.h"
#include "AssetDescriptor.h"

#include <fstream>
#include <utility>

FileAssetDescriptor::FileAssetDescriptor(std::string path, std::string name)
        : mPath(std::move(path)), mName(std::move(name)) {}

std::unique_ptr<std::istream> FileAssetDescriptor::getStream() const {
    auto file = std::make_unique<std::ifstream>(mPath, std::ios::binary);
    if (file->fail()) {
        throw std::runtime_error("Asset file \"" + mPath + "\" not found.");
    }
    return file;
}

std::vector<uint8_t> FileAssetDescriptor::getBytes() const {
    std::ifstream file(mPath, std::ios::binary);
    if (file.fail()) {
        throw std::runtime_error("Asset file \"" + mPath + "\" not found.");
    }
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

std::string FileAssetDescriptor::getPath() const {
    return mPath;
}

std::string FileAssetDescriptor::getName() const {
    return mName;
}
