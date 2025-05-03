#include "pch.h"
#include "Mesh.h"

SubMesh::SubMesh(uint32_t materialIndex, uint32_t firstIndex, uint32_t indexCount)
        : materialIndex(materialIndex), firstIndex(firstIndex), indexCount(indexCount) {}

VertexInputLayout Mesh::getVertexInputLayout() {
    static vk::VertexInputBindingDescription binding(
            0,
            sizeof(MeshVertex),
            vk::VertexInputRate::eVertex
    );

    static vk::VertexInputAttributeDescription position(
            0,
            0,
            vk::Format::eR32G32B32Sfloat,
            0
    );

    static vk::VertexInputAttributeDescription normal(
            1,
            0,
            vk::Format::eR32G32B32Sfloat,
            offsetof(MeshVertex, normal)
    );

    static vk::VertexInputAttributeDescription tangent(
            2,
            0,
            vk::Format::eR32G32B32Sfloat,
            offsetof(MeshVertex, tangent)
    );

    static vk::VertexInputAttributeDescription uv0(
            3,
            0,
            vk::Format::eR32G32Sfloat,
            offsetof(MeshVertex, uv0)
    );

    static vk::VertexInputAttributeDescription uv1(
            4,
            0,
            vk::Format::eR32G32Sfloat,
            offsetof(MeshVertex, uv1)
    );

    static VertexInputLayout vertexInputLayout{
            {binding},
            {position, normal, tangent, uv0, uv1}
    };

    return vertexInputLayout;
}

void Mesh::updateBuffers(const VulkanContext& context) {
    mVertices.shrink_to_fit();
    mIndices.shrink_to_fit();

    uint32_t queueFamilyIndex = context.getMainQueueFamilyIndex();

    uint64_t verticesSize = mVertices.size() * sizeof(decltype(mVertices)::value_type);
    uint64_t indicesSize = mIndices.size() * sizeof(decltype(mIndices)::value_type);

    vk::BufferCreateInfo vboCreateInfo(
            {},
            verticesSize,
            {vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst},
            vk::SharingMode::eExclusive,
            1, &queueFamilyIndex
    );

    vk::BufferCreateInfo iboCreateInfo(
            {},
            indicesSize,
            {vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst},
            vk::SharingMode::eExclusive,
            1, &queueFamilyIndex
    );

    mVertexBuffer = std::make_unique<AllocatedBuffer>(context, vboCreateInfo);
    mIndexBuffer = std::make_unique<AllocatedBuffer>(context, iboCreateInfo);

    mVertexBuffer->uploadData(static_cast<const void*>(mVertices.data()), 0, verticesSize);
    mIndexBuffer->uploadData(static_cast<const void*>(mIndices.data()), 0, indicesSize);
}