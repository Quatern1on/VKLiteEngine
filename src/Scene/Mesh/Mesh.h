#pragma once
#include "pch.h"

#include "Scene/Mesh/Material.h"
#include "Vulkan/Allocator/AllocatedBuffer.h"

struct VertexInputLayout {
    std::vector<vk::VertexInputBindingDescription> bindings;
    std::vector<vk::VertexInputAttributeDescription> attributes;
};

struct MeshVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 uv0;
    glm::vec2 uv1;
};

struct SubMesh {
    explicit SubMesh() = default;

    explicit SubMesh(uint32_t materialIndex, uint32_t firstIndex, uint32_t indexCount);

    uint32_t firstIndex = 0;
    uint32_t indexCount = 0;

    uint32_t materialIndex = 0;
};

class Mesh {
public:
    static VertexInputLayout getVertexInputLayout();

    explicit Mesh() = default;

    Mesh(const Mesh&) = delete;

    Mesh& operator=(const Mesh&) = delete;

    void updateBuffers(const VulkanContext& context);

    inline const std::vector<MeshVertex>& getVertices() const {
        return mVertices;
    }

    inline std::vector<MeshVertex>& getVertices() {
        return mVertices;
    }

    inline const std::vector<uint32_t>& getIndices() const {
        return mIndices;
    }

    inline std::vector<uint32_t>& getIndices() {
        return mIndices;
    }

    inline const std::vector<SubMesh>& getSubMeshes() const {
        return mSubMeshes;
    }

    inline std::vector<SubMesh>& getSubMeshes() {
        return mSubMeshes;
    }

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

    inline std::vector<std::shared_ptr<Material>>& getMaterials() {
        return mMaterials;
    }

    inline const std::vector<std::shared_ptr<Material>>& getMaterials() const {
        return mMaterials;
    }

private:
    std::vector<MeshVertex> mVertices;
    std::vector<uint32_t> mIndices;
    std::vector<SubMesh> mSubMeshes;

    std::unique_ptr<AllocatedBuffer> mVertexBuffer;
    std::unique_ptr<AllocatedBuffer> mIndexBuffer;

    std::vector<std::shared_ptr<Material>> mMaterials;
};
