#include "pch.h"
#include "MeshLoader.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

std::shared_ptr<Mesh> MeshLoader::loadAsSingleMesh(std::unique_ptr<AssetDescriptor> modelAsset) {
    Assimp::Importer importer;

    std::vector<uint8_t> modelBytes = modelAsset->getBytes();

    const aiScene* importedScene = importer.ReadFileFromMemory(modelBytes.data(), modelBytes.size(),
            aiProcess_Triangulate |
            aiProcess_MakeLeftHanded |
            aiProcess_GenNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace |
            aiProcess_OptimizeMeshes |
            aiProcess_JoinIdenticalVertices |
            aiProcess_PreTransformVertices |
            aiProcess_SortByPType);

    auto resultMesh = std::make_shared<Mesh>();

    for (int i = 0; i < importedScene->mNumMaterials; i++) {
        aiMaterial* material = importedScene->mMaterials[i];
        LOG(INFO) << modelAsset->getName() << ": " << material->GetName().C_Str();
    }

    for (int meshIndex = 0; meshIndex < importedScene->mNumMeshes; meshIndex++) {
        aiMesh* importedMesh = importedScene->mMeshes[meshIndex];

        if ((importedMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) != aiPrimitiveType_TRIANGLE) {
            continue;
        }

        uint32_t indexOffset = resultMesh->getVertices().size();
        uint32_t firstIndex = resultMesh->getIndices().size();

        for (int vertexIndex = 0; vertexIndex < importedMesh->mNumVertices; vertexIndex++) {
            MeshVertex newVertex{};
            newVertex.position = glm::vec3(importedMesh->mVertices[vertexIndex].x,
                    importedMesh->mVertices[vertexIndex].y,
                    importedMesh->mVertices[vertexIndex].z);

            if (importedMesh->HasNormals()) {
                newVertex.normal = glm::vec3(importedMesh->mNormals[vertexIndex].x,
                        importedMesh->mNormals[vertexIndex].y,
                        importedMesh->mNormals[vertexIndex].z);
            }

            if (importedMesh->HasTangentsAndBitangents()) {
                newVertex.tangent = glm::vec3(importedMesh->mTangents[vertexIndex].x,
                        importedMesh->mTangents[vertexIndex].y,
                        importedMesh->mTangents[vertexIndex].z);
            }

            aiVector3D* importedUV0 = importedMesh->mTextureCoords[0];
            aiVector3D* importedUV1 = importedMesh->mTextureCoords[1];

            if (importedUV0) {
                newVertex.uv0 = glm::vec2(importedUV0[vertexIndex].x, importedUV0[vertexIndex].y);
            }
            if (importedUV1) {
                newVertex.uv1 = glm::vec2(importedUV1[vertexIndex].x, importedUV1[vertexIndex].y);
            }

            resultMesh->getVertices().push_back(newVertex);
        }

        for (int faceIndex = 0; faceIndex < importedMesh->mNumFaces; faceIndex++) {
            aiFace importedFace = importedMesh->mFaces[faceIndex];

            resultMesh->getIndices().push_back(importedFace.mIndices[0] + indexOffset);
            resultMesh->getIndices().push_back(importedFace.mIndices[1] + indexOffset);
            resultMesh->getIndices().push_back(importedFace.mIndices[2] + indexOffset);
        }

        resultMesh->getSubMeshes().emplace_back(importedMesh->mMaterialIndex, firstIndex, importedMesh->mNumFaces * 3);
    }

    for (int i = 0; i < importedScene->mNumMaterials; i++) {
        resultMesh->getMaterials().push_back(Material::kDefaultMaterial);
    }

    resultMesh->updateBuffers(Engine::getInstance().getVulkanContext());

    return resultMesh;
}
