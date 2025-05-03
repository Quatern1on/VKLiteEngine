#pragma once
#include "pch.h"

class AssetDescriptor {
public:
    virtual ~AssetDescriptor() = default;

    virtual std::unique_ptr<std::istream> getStream() const = 0;

    virtual std::vector<uint8_t> getBytes() const = 0;

    virtual std::string getPath() const = 0;

    virtual std::string getName() const = 0;
};

class FileAssetDescriptor : public AssetDescriptor {
public:
    explicit FileAssetDescriptor(std::string path, std::string name);

    std::unique_ptr<std::istream> getStream() const override;

    std::vector<uint8_t> getBytes() const override;

    std::string getPath() const override;

    std::string getName() const override;

private:
    const std::string mName;

    const std::string mPath;
};