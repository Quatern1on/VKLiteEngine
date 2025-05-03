#pragma once
#include "pch.h"

class LightVolumeMesh {
public:
    static VertexInputLayout getVertexInputLayout();

    static std::unique_ptr<LightVolumeMesh> load(std::unique_ptr<AssetDescriptor> meshAsset);

    LightVolumeMesh(const VulkanContext& context, const std::vector<glm::vec3>& vertices,
            const std::vector<uint32_t>& indices);

    LightVolumeMesh(const LightVolumeMesh&) = delete;

    LightVolumeMesh& operator=(const LightVolumeMesh&) = delete;

    inline const AllocatedBuffer& getVertexBuffer() const {
        return *mVertexBuffer;
    }

    inline AllocatedBuffer& getVertexBuffer() {
        return *mVertexBuffer;
    }

    inline const AllocatedBuffer& getIndexBuffer() const {
        return *mIndexBuffer;
    }

    inline AllocatedBuffer& getIndexBuffer() {
        return *mIndexBuffer;
    }

    inline uint32_t getIndexCount() const {
        return mIndexCount;
    }

private:
    std::unique_ptr<AllocatedBuffer> mVertexBuffer;
    std::unique_ptr<AllocatedBuffer> mIndexBuffer;

    uint32_t mIndexCount = 0;
};
