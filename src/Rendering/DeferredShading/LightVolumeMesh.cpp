#include "pch.h"
#include "LightVolumeMesh.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

VertexInputLayout LightVolumeMesh::getVertexInputLayout() {
    static vk::VertexInputBindingDescription binding(
            0,
            sizeof(glm::vec3),
            vk::VertexInputRate::eVertex
    );

    static vk::VertexInputAttributeDescription vertex(
            0,
            0,
            vk::Format::eR32G32B32Sfloat,
            0
    );

    static VertexInputLayout vertexInputLayout{
            {binding},
            {vertex}
    };

    return vertexInputLayout;
}

std::unique_ptr<LightVolumeMesh> LightVolumeMesh::load(std::unique_ptr<AssetDescriptor> meshAsset) {
    Assimp::Importer importer;

    std::vector<uint8_t> modelBytes = meshAsset->getBytes();

    const aiScene* importedScene = importer.ReadFileFromMemory(modelBytes.data(), modelBytes.size(),
            aiProcess_Triangulate |
            aiProcess_MakeLeftHanded |
            aiProcess_OptimizeMeshes |
            aiProcess_JoinIdenticalVertices |
            aiProcess_PreTransformVertices |
            aiProcess_SortByPType);

    if (importedScene->mNumMeshes != 1) {
        throw std::runtime_error(
                "Asset is not suitable for being a light volume mesh as it contains more that one mesh");
    }

    aiMesh* importedMesh = importedScene->mMeshes[0];

    if ((importedMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) != aiPrimitiveType_TRIANGLE) {
        throw std::runtime_error("Mesh does not contain any triangles");
    }

    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(importedMesh->mNumVertices);
    vertices.reserve(importedMesh->mNumFaces * 3);

    for (int vertexIndex = 0; vertexIndex < importedMesh->mNumVertices; vertexIndex++) {
        glm::vec3 vertex = glm::vec3(importedMesh->mVertices[vertexIndex].x,
                importedMesh->mVertices[vertexIndex].y,
                importedMesh->mVertices[vertexIndex].z);

        vertices.push_back(vertex);
    }

    for (int faceIndex = 0; faceIndex < importedMesh->mNumFaces; faceIndex++) {
        aiFace importedFace = importedMesh->mFaces[faceIndex];

        indices.push_back(importedFace.mIndices[0]);
        indices.push_back(importedFace.mIndices[1]);
        indices.push_back(importedFace.mIndices[2]);
    }

    return std::make_unique<LightVolumeMesh>(Engine::getInstance().getVulkanContext(), vertices, indices);
}

LightVolumeMesh::LightVolumeMesh(const VulkanContext& context, const std::vector<glm::vec3>& vertices,
        const std::vector<uint32_t>& indices) {
    uint32_t queueFamilyIndex = context.getMainQueueFamilyIndex();

    uint64_t verticesSize = vertices.size() * sizeof(std::remove_reference<decltype(vertices)>::type::value_type);
    uint64_t indicesSize = indices.size() * sizeof(std::remove_reference<decltype(indices)>::type::value_type);

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

    mVertexBuffer->uploadData(static_cast<const void*>(vertices.data()), 0, verticesSize);
    mIndexBuffer->uploadData(static_cast<const void*>(indices.data()), 0, indicesSize);

    mIndexCount = indices.size();
}
