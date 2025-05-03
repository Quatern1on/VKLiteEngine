#include "pch.h"
#include "GeometryPass.h"

#include "Scene/Mesh/Mesh.h"

GeometryPass::GeometryPass(const VulkanContext& context, const DeferredShading& deferredShading)
        : mContext(std::ref(context)), mDeferredShading(std::ref(deferredShading)) {
    createDescriptorGenerators();
    createPipeline();

    createDefaultTextures();

    mUBO = std::make_unique<UniformBuffer>(context, 32 * 1024 * 1024); //32 megabytes
}

void GeometryPass::createDescriptorGenerators() {
    std::vector<vk::DescriptorSetLayoutBinding> objectDescriptorBindings;
    std::vector<vk::DescriptorSetLayoutBinding> materialDescriptorBindings;

    objectDescriptorBindings.emplace_back(0,
            vk::DescriptorType::eUniformBuffer,
            1,
            vk::ShaderStageFlags{vk::ShaderStageFlagBits::eVertex},
            nullptr
    );

    materialDescriptorBindings.emplace_back(
            0,
            vk::DescriptorType::eUniformBuffer,
            1,
            vk::ShaderStageFlags{vk::ShaderStageFlagBits::eFragment},
            nullptr
    );

    for (int i = 1; i <= 6; i++) {
        materialDescriptorBindings.emplace_back(
                i,
                vk::DescriptorType::eCombinedImageSampler,
                1,
                vk::ShaderStageFlags{vk::ShaderStageFlagBits::eFragment},
                nullptr
        );
    }

    mObjectDescriptorGenerator = std::make_unique<DescriptorGenerator>(mContext.get(), objectDescriptorBindings);
    mMaterialDescriptorGenerator = std::make_unique<DescriptorGenerator>(mContext.get(), materialDescriptorBindings);
}

void GeometryPass::createPipeline() {
    const GBuffer& gBuffer = mDeferredShading.get().getGBuffer();
    uint32_t width = gBuffer.getWidth();
    uint32_t height = gBuffer.getHeight();

    vk::raii::ShaderModule vertexShaderModule = VulkanUtils::createShaderModule(mContext,
            *AssetFactory::getShaderAsset("deferred/geometry.vert"));
    vk::raii::ShaderModule fragmentShaderModule = VulkanUtils::createShaderModule(mContext,
            *AssetFactory::getShaderAsset("deferred/geometry.frag"));

    vk::PipelineShaderStageCreateInfo vertexShaderStage(
            {},
            vk::ShaderStageFlagBits::eVertex,
            *vertexShaderModule,
            "main",
            nullptr
    );
    vk::PipelineShaderStageCreateInfo fragmentShaderStage(
            {},
            vk::ShaderStageFlagBits::eFragment,
            *fragmentShaderModule,
            "main",
            nullptr
    );
    std::array<vk::PipelineShaderStageCreateInfo, 2> stages = {vertexShaderStage, fragmentShaderStage};

    VertexInputLayout inputLayout = Mesh::getVertexInputLayout();

    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo(
            {},
            inputLayout.bindings,
            inputLayout.attributes
    );

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo(
            {},
            vk::PrimitiveTopology::eTriangleList,
            false
    );

    vk::Viewport viewport(
            0, 0,
            static_cast<float>(width), static_cast<float>(height),
            0.0f, 1.0f
    );
    std::array<vk::Viewport, 1> viewports{viewport};

    vk::Rect2D scissor(
            {0, 0},
            {width, height}
    );
    std::array<vk::Rect2D, 1> scissors{scissor};

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo(
            {},
            viewports,
            scissors
    );

    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(
            {},
            false,
            false,
            vk::PolygonMode::eFill,
            {vk::CullModeFlagBits::eBack},
            vk::FrontFace::eCounterClockwise,
            false,
            0.0f,
            0.0f,
            0.0f,
            1.0f
    );

    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo(
            {},
            vk::SampleCountFlagBits::e1,
            false,
            1.0f,
            nullptr,
            false,
            false
    );

    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo(
            {},
            true,
            true,
            vk::CompareOp::eLess,
            false,
            false,
            {},
            {},
            0.0f,
            1.0f
    );

    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
    colorBlendAttachments.reserve(gBuffer.size() + 1);

    for (int i = 0; i < gBuffer.size() + 1; i++) {
        colorBlendAttachments.emplace_back(
                false,
                vk::BlendFactor::eOne,
                vk::BlendFactor::eZero,
                vk::BlendOp::eAdd,
                vk::BlendFactor::eOne,
                vk::BlendFactor::eZero,
                vk::BlendOp::eAdd,
                vk::ColorComponentFlags{vk::ColorComponentFlagBits::eR |
                                        vk::ColorComponentFlagBits::eG |
                                        vk::ColorComponentFlagBits::eB |
                                        vk::ColorComponentFlagBits::eA}
        );
    }

    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo(
            {},
            false,
            vk::LogicOp::eCopy,
            colorBlendAttachments,
            {0.0f, 0.0f, 0.0f, 0.0f}
    );

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{*mMaterialDescriptorGenerator->getLayout(),
                                                              *mObjectDescriptorGenerator->getLayout()};
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            {},
            descriptorSetLayouts,
            {}
    );

    mPipelineLayout = std::make_unique<vk::raii::PipelineLayout>(mContext.get().getDevice(),
            pipelineLayoutCreateInfo);

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo(
            {},
            stages,
            &vertexInputStateCreateInfo,
            &inputAssemblyStateCreateInfo,
            nullptr,
            &viewportStateCreateInfo,
            &rasterizationStateCreateInfo,
            &multisampleStateCreateInfo,
            &depthStencilStateCreateInfo,
            &colorBlendStateCreateInfo,
            nullptr,
            **mPipelineLayout,
            *mDeferredShading.get().getRenderPass(),
            0,
            nullptr,
            0
    );

    mPipeline = std::make_unique<vk::raii::Pipeline>(mContext.get().getDevice(), nullptr, pipelineCreateInfo);
}

void GeometryPass::createDefaultTextures() {
    uint32_t one = 0xffffffffU;
    uint32_t zero = 0x0U;
    glm::vec<4, uint8_t, glm::mediump> normal{};
    normal.x = 0x80U;
    normal.y = 0x80U;
    normal.z = 0xffU;
    normal.w = 0xFFU;

    mDefaultOneTexture = std::make_unique<Texture2D>(mContext, 1, 1, vk::Format::eR8G8B8A8Unorm,
            false, &one, 4, "default_one");

    mDefaultOneSmallTexture = std::make_unique<Texture2D>(mContext, 1, 1, vk::Format::eR8Unorm,
            false, &one, 1, "default_one_small");

    mDefaultZeroTexture = std::make_unique<Texture2D>(mContext, 1, 1, vk::Format::eR8Unorm,
            false, &zero, 1, "default_zero");

    mDefaultNormalTexture = std::make_unique<Texture2D>(mContext, 1, 1, vk::Format::eR8G8B8A8Unorm,
            false, &normal, 4, "default_normal");
}

void GeometryPass::render(vk::raii::CommandBuffer& commandBuffer,
        const std::vector<std::reference_wrapper<MeshComponent>>& meshes, const CameraComponent& camera) {
    BEGIN_DEBUG_LABEL(commandBuffer, "Geometry Pass", glm::vec3(0.0f, 1.0f, 0.0f));

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **mPipeline);

    mUBO->restart();

    std::unordered_map<std::shared_ptr<Material>, uint64_t> materialIndices;
    std::vector<vk::DescriptorSet> materialDescriptors;

    writeMaterialDescriptors(meshes, materialIndices, materialDescriptors);

    std::vector<vk::DescriptorSet> objectDescriptors;

    writeObjectDescriptors(meshes, objectDescriptors, camera);

    for (uint32_t meshIndex = 0; meshIndex < meshes.size(); meshIndex++) {
        const MeshComponent& mesh = meshes[meshIndex].get();

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **mPipelineLayout, 1,
                {objectDescriptors[meshIndex]}, {});

        commandBuffer.bindVertexBuffers(0, {*mesh.getMesh().getVertexBuffer()}, {0});
        commandBuffer.bindIndexBuffer(*mesh.getMesh().getIndexBuffer(), 0, vk::IndexType::eUint32);

        for (const auto& subMesh : mesh.getMesh().getSubMeshes()) {
            uint32_t materialIndex = materialIndices[mesh.getMaterialSlots()[subMesh.materialIndex]];

            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **mPipelineLayout, 0,
                    {materialDescriptors[materialIndex]}, {});

            commandBuffer.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
        }
    }

    END_DEBUG_LABEL(commandBuffer);
}

void GeometryPass::writeMaterialDescriptors(const std::vector<std::reference_wrapper<MeshComponent>>& meshes,
        std::unordered_map<std::shared_ptr<Material>, uint64_t>& materialIndices,
        std::vector<vk::DescriptorSet>& materialDescriptors) {
    uint32_t materialsCount = 0;

    for (auto mesh : meshes) {
        for (const auto& subMesh : mesh.get().getMesh().getSubMeshes()) {
            if (!materialIndices.contains(mesh.get().getMaterialSlots()[subMesh.materialIndex])) {
                materialIndices[mesh.get().getMaterialSlots()[subMesh.materialIndex]] = materialsCount++;
            }
        }
    }

    mMaterialDescriptorGenerator->restart(materialsCount * 3);
    materialDescriptors = mMaterialDescriptorGenerator->createDescriptorSets(materialsCount);

    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    std::vector<vk::DescriptorImageInfo> imageInfos;
    std::vector<vk::WriteDescriptorSet> descriptorWrites;

    bufferInfos.reserve(materialsCount);
    imageInfos.reserve(materialsCount * 6);
    descriptorWrites.reserve(materialsCount * 7);

    for (auto [material, materialIndex] : materialIndices) {
        MaterialUniform materialUniform{material->albedo,
                                        material->emissive,
                                        material->metallic,
                                        material->roughness};

        uint64_t offset = mUBO->pushData(&materialUniform, sizeof(MaterialUniform));

        bufferInfos.emplace_back(**mUBO, offset, sizeof(MaterialUniform));
        descriptorWrites.emplace_back(
                materialDescriptors[materialIndex],
                0,
                0,
                1,
                vk::DescriptorType::eUniformBuffer,
                nullptr,
                &bufferInfos.back(),
                nullptr
        );

        Texture2D& albedoTexture = (material->albedoTexture) ? *material->albedoTexture : *mDefaultOneTexture;
        Texture2D& emissiveTexture = (material->emissiveTexture) ? *material->emissiveTexture : *mDefaultOneTexture;
        Texture2D& normalTexture = (material->normalTexture) ? *material->normalTexture : *mDefaultNormalTexture;
        Texture2D& metallicTexture = (material->metallicTexture) ? *material->metallicTexture
                                                                 : *mDefaultOneSmallTexture;
        Texture2D& roughnessTexture = (material->roughnessTexture) ? *material->roughnessTexture
                                                                   : *mDefaultOneSmallTexture;
        Texture2D& aoTexture = (material->aoTexture) ? *material->aoTexture : *mDefaultOneSmallTexture;

        imageInfos.emplace_back(*albedoTexture.getSampler(), *albedoTexture.getImageView(),
                vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfos.emplace_back(*emissiveTexture.getSampler(), *emissiveTexture.getImageView(),
                vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfos.emplace_back(*normalTexture.getSampler(), *normalTexture.getImageView(),
                vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfos.emplace_back(*metallicTexture.getSampler(), *metallicTexture.getImageView(),
                vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfos.emplace_back(*roughnessTexture.getSampler(), *roughnessTexture.getImageView(),
                vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfos.emplace_back(*aoTexture.getSampler(), *aoTexture.getImageView(),
                vk::ImageLayout::eShaderReadOnlyOptimal);

        for (int i = 1; i <= 6; i++) {
            descriptorWrites.emplace_back(
                    materialDescriptors[materialIndex],
                    i,
                    0,
                    1,
                    vk::DescriptorType::eCombinedImageSampler,
                    &imageInfos[imageInfos.size() - 7 + i],
                    nullptr,
                    nullptr
            );
        }
    }

    mContext.get().getDevice().updateDescriptorSets(descriptorWrites, {});
}

void GeometryPass::writeObjectDescriptors(const std::vector<std::reference_wrapper<MeshComponent>>& meshes,
        std::vector<vk::DescriptorSet>& objectDescriptors, const CameraComponent& camera) {
    uint32_t objectsCount = meshes.size();

    mObjectDescriptorGenerator->restart(objectsCount);
    objectDescriptors = mObjectDescriptorGenerator->createDescriptorSets(objectsCount);

    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    std::vector<vk::WriteDescriptorSet> descriptorWrites;

    bufferInfos.reserve(objectsCount);
    descriptorWrites.reserve(objectsCount);

    glm::mat4 viewMatrix = camera.getViewMatrix();
    glm::mat4 projectionMatrix = camera.getCamera().getProjectionMatrix();

    for (uint32_t i = 0; i < objectsCount; i++) {
        std::reference_wrapper<MeshComponent> mesh = meshes[i];

        glm::mat4 modelMatrix = mesh.get().getSceneObject()->getTransform().getLocalToWorldMatrix();
        glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
        glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(modelMatrix));
        glm::mat4 modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;

        ObjectUniform objectUniform{
                modelMatrix,
                normalMatrix,
                modelViewProjectionMatrix
        };

        uint64_t offset = mUBO->pushData(&objectUniform, sizeof(ObjectUniform));
        bufferInfos.emplace_back(**mUBO, offset, sizeof(ObjectUniform));
        descriptorWrites.emplace_back(
                objectDescriptors[i],
                0,
                0,
                1,
                vk::DescriptorType::eUniformBuffer,
                nullptr,
                &bufferInfos.back(),
                nullptr
        );
    }

    mContext.get().getDevice().updateDescriptorSets(descriptorWrites, {});
}
