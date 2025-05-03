#include "pch.h"
#include <Scene/Light/PointLightComponent.h>
#include "LightPass.h"

LightPass::LightPass(const VulkanContext& context, const DeferredShading& deferredShading,
        const FrameRenderer& frameRenderer)
        : mContext(std::ref(context)), mDeferredShading(std::ref(deferredShading)) {
    createSubpassInputDescriptor(frameRenderer);
    createDescriptorGenerators();
    createPipeline();

    mUBO = std::make_unique<UniformBuffer>(context, 32 * 1024 * 1024); //32 megabytes

    mPointLightVolumeMesh = LightVolumeMesh::load(AssetFactory::getModelAsset("point_light_volume.obj"));
}

void LightPass::createDescriptorGenerators() {
    std::vector<vk::DescriptorSetLayoutBinding> frameDescriptorBindings;
    std::vector<vk::DescriptorSetLayoutBinding> objectDescriptorBindings;

    vk::DescriptorSetLayoutBinding frameBinding(0,
            vk::DescriptorType::eUniformBuffer,
            1,
            {vk::ShaderStageFlagBits::eFragment},
            nullptr
    );
    frameDescriptorBindings.push_back(frameBinding);

    vk::DescriptorSetLayoutBinding lightVolumeUniform(0,
            vk::DescriptorType::eUniformBuffer,
            1,
            {vk::ShaderStageFlagBits::eVertex},
            nullptr
    );
    objectDescriptorBindings.push_back(lightVolumeUniform);

    vk::DescriptorSetLayoutBinding objectBinding(1,
            vk::DescriptorType::eUniformBuffer,
            1,
            {vk::ShaderStageFlagBits::eFragment},
            nullptr
    );
    objectDescriptorBindings.push_back(objectBinding);

    mFrameDescriptorGenerator = std::make_unique<DescriptorGenerator>(mContext.get(), frameDescriptorBindings, 1);
    mObjectDescriptorGenerator = std::make_unique<DescriptorGenerator>(mContext.get(), objectDescriptorBindings);
}

void LightPass::createPipeline() {
    const GBuffer& gBuffer = mDeferredShading.get().getGBuffer();
    uint32_t width = gBuffer.getWidth();
    uint32_t height = gBuffer.getHeight();

    vk::raii::ShaderModule vertexShaderModule = VulkanUtils::createShaderModule(mContext,
            *AssetFactory::getShaderAsset("deferred/point_light.vert"));
    vk::raii::ShaderModule fragmentShaderModule = VulkanUtils::createShaderModule(mContext,
            *AssetFactory::getShaderAsset("deferred/point_light.frag"));

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

    VertexInputLayout inputLayout = LightVolumeMesh::getVertexInputLayout();

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
            {vk::CullModeFlagBits::eFront},
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
            false,
            vk::CompareOp::eGreater,
            false,
            false,
            {},
            {},
            0.0f,
            1.0f
    );

    vk::PipelineColorBlendAttachmentState colorBlendAttachment(
            true,
            vk::BlendFactor::eOne,
            vk::BlendFactor::eOne,
            vk::BlendOp::eAdd,
            vk::BlendFactor::eOne,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            vk::ColorComponentFlags{vk::ColorComponentFlagBits::eR |
                                    vk::ColorComponentFlagBits::eG |
                                    vk::ColorComponentFlagBits::eB}
    );

    std::array<vk::PipelineColorBlendAttachmentState, 1> colorBlendAttachments{colorBlendAttachment};

    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo(
            {},
            false,
            vk::LogicOp::eCopy,
            colorBlendAttachments,
            {0.0f, 0.0f, 0.0f, 0.0f}
    );

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{*mSubpassInputDescriptorGenerator->getLayout(),
                                                              *mFrameDescriptorGenerator->getLayout(),
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
            1,
            nullptr,
            0
    );

    mPipeline = std::make_unique<vk::raii::Pipeline>(mContext.get().getDevice(), nullptr, pipelineCreateInfo);
}

void LightPass::render(vk::raii::CommandBuffer& commandBuffer, const Scene& scene) {
    BEGIN_DEBUG_LABEL(commandBuffer, "Light Pass", glm::vec3(1.0f, 0.0f, 0.0f));

    BEGIN_DEBUG_LABEL(commandBuffer, "Point Lights", glm::vec3(0.5f, 0.0f, 0.0f));

    std::vector<std::reference_wrapper<PointLightComponent>> pointLights =
            scene.getRoot().getComponentsRecursive<PointLightComponent>();

    mUBO->restart();

    vk::DescriptorSet frameDescriptor = writeFrameDescriptors(*scene.getCamera());

    std::vector<vk::DescriptorSet> objectDescriptors;
    writeObjectDescriptors(pointLights, objectDescriptors, *scene.getCamera());

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **mPipeline);

    commandBuffer.bindVertexBuffers(0, {*mPointLightVolumeMesh->getVertexBuffer()}, {0});
    commandBuffer.bindIndexBuffer(*mPointLightVolumeMesh->getIndexBuffer(), 0, vk::IndexType::eUint32);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **mPipelineLayout, 0,
            {mSubpassInputDescriptorSet, frameDescriptor}, {});

    for (uint32_t i = 0; i < pointLights.size(); i++) {
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **mPipelineLayout, 2,
                {objectDescriptors[i]}, {});

        commandBuffer.drawIndexed(mPointLightVolumeMesh->getIndexCount(), 1, 0, 0, 0);
    }

    END_DEBUG_LABEL(commandBuffer);
    END_DEBUG_LABEL(commandBuffer);
}

void LightPass::createSubpassInputDescriptor(const FrameRenderer& frameRenderer) {
    std::vector<vk::DescriptorSetLayoutBinding> subpassInputDescriptorBindings;

    int gBufferSize = mDeferredShading.get().getGBuffer().size();

    for (int i = 0; i < gBufferSize; i++) {
        vk::DescriptorSetLayoutBinding inputBinding(
                i,
                vk::DescriptorType::eInputAttachment,
                1,
                {vk::ShaderStageFlagBits::eFragment},
                nullptr
        );
        subpassInputDescriptorBindings.push_back(inputBinding);
    }

    vk::DescriptorSetLayoutBinding inputBinding(
            gBufferSize,
            vk::DescriptorType::eInputAttachment,
            1,
            {vk::ShaderStageFlagBits::eFragment},
            nullptr
    );
    subpassInputDescriptorBindings.push_back(inputBinding);

    mSubpassInputDescriptorGenerator = std::make_unique<DescriptorGenerator>(mContext.get(),
            subpassInputDescriptorBindings, 1);

    mSubpassInputDescriptorGenerator->restart(1);
    mSubpassInputDescriptorSet = mSubpassInputDescriptorGenerator->createDescriptorSet();

    std::vector<vk::DescriptorImageInfo> imageInfos;
    std::vector<vk::WriteDescriptorSet> descriptorWrites;
    imageInfos.reserve(gBufferSize + 1);
    descriptorWrites.reserve(gBufferSize + 1);

    for (int i = 0; i < gBufferSize; i++) {
        imageInfos.emplace_back(
                vk::Sampler(nullptr),
                *mDeferredShading.get().getGBuffer().getImageView(i),
                vk::ImageLayout::eShaderReadOnlyOptimal
        );
        descriptorWrites.emplace_back(
                mSubpassInputDescriptorSet,
                i,
                0,
                1,
                vk::DescriptorType::eInputAttachment,
                &imageInfos.back(),
                nullptr,
                nullptr
        );
    }

    imageInfos.emplace_back(
            vk::Sampler(nullptr),
            *frameRenderer.getMainDepthImageView(),
            vk::ImageLayout::eDepthStencilReadOnlyOptimal
    );
    descriptorWrites.emplace_back(
            mSubpassInputDescriptorSet,
            gBufferSize,
            0,
            1,
            vk::DescriptorType::eInputAttachment,
            &imageInfos.back(),
            nullptr,
            nullptr
    );

    mContext.get().getDevice().updateDescriptorSets(descriptorWrites, {});
}

vk::DescriptorSet LightPass::writeFrameDescriptors(const CameraComponent& camera) {
    mFrameDescriptorGenerator->restart(1);
    vk::DescriptorSet set = mFrameDescriptorGenerator->createDescriptorSet();

    glm::mat4 viewMatrix = camera.getViewMatrix();
    glm::mat4 projectionMatrix = camera.getCamera().getProjectionMatrix();
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
    glm::vec2 screenSize = glm::vec2(camera.getCamera().getWidth(), camera.getCamera().getHeight());
    FrameUniform frameUniform{
            camera.getSceneObject()->getTransform().getPosition(),
            glm::inverse(viewProjectionMatrix),
            screenSize
    };

    uint64_t offset = mUBO->pushData(&frameUniform, sizeof(FrameUniform));

    vk::DescriptorBufferInfo bufferInfo(**mUBO, offset, sizeof(FrameUniform));

    vk::WriteDescriptorSet descriptorWrite(
            set,
            0,
            0,
            1,
            vk::DescriptorType::eUniformBuffer,
            nullptr,
            &bufferInfo,
            nullptr
    );

    mContext.get().getDevice().updateDescriptorSets({descriptorWrite}, {});

    return set;
}

void LightPass::writeObjectDescriptors(const std::vector<std::reference_wrapper<PointLightComponent>>& pointLights,
        std::vector<vk::DescriptorSet>& objectDescriptors, const CameraComponent& camera) {
    uint32_t objectsCount = pointLights.size();

    mObjectDescriptorGenerator->restart(objectsCount);
    objectDescriptors = mObjectDescriptorGenerator->createDescriptorSets(objectsCount);

    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    std::vector<vk::WriteDescriptorSet> descriptorWrites;

    bufferInfos.reserve(objectsCount * 2);
    descriptorWrites.reserve(objectsCount * 2);

    glm::mat4 viewMatrix = camera.getViewMatrix();
    glm::mat4 projectionMatrix = camera.getCamera().getProjectionMatrix();
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

    for (uint32_t i = 0; i < objectsCount; i++) {
        std::reference_wrapper<PointLightComponent> pointLight = pointLights[i];

        glm::vec3 position = pointLight.get().getSceneObject()->getTransform().getPosition();

        //Light volume uniform
        float radius = pointLight.get().getRadius();

        glm::mat4 modelViewProjectionMatrix = glm::translate(viewProjectionMatrix, position);
        modelViewProjectionMatrix = glm::scale(modelViewProjectionMatrix, glm::vec3(radius * 1.03f));
        LightVolumeUniform lightVolumeUniform{
                modelViewProjectionMatrix
        };

        uint64_t offset = mUBO->pushData(&lightVolumeUniform, sizeof(LightVolumeUniform));
        bufferInfos.emplace_back(**mUBO, offset, sizeof(LightVolumeUniform));
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

        //Point light fragment uniform
        glm::vec3 intensity = pointLight.get().getIntensity();

        PointLightFragmentUniform pointLightUniform{
                position,
                intensity,
                pointLight.get().getRadius()
        };

        offset = mUBO->pushData(&pointLightUniform, sizeof(PointLightFragmentUniform));
        bufferInfos.emplace_back(**mUBO, offset, sizeof(PointLightFragmentUniform));
        descriptorWrites.emplace_back(
                objectDescriptors[i],
                1,
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
