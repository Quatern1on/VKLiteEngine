#include "pch.h"
#include "Bloom.h"

Bloom::Bloom(const VulkanContext& context, const FrameRenderer& frameRenderer,
        uint32_t width, uint32_t height)
        : mContext(std::ref(context)), mMainColorImageView(std::ref(frameRenderer.getMainColorImageView())),
          mFrameRenderer(std::ref(frameRenderer)), mWidth(width), mHeight(height) {
    createImages();
    createRenderPasses();
    createFramebuffers();
    createTextureDescriptors();
    createPipelines();
}

void Bloom::render(vk::raii::CommandBuffer& commandBuffer) {
    BEGIN_DEBUG_LABEL(commandBuffer, "Bloom", glm::vec3(1.0f, 1.0f, 1.0f));

    for (int i = 0; i < 1; i++) {
        commandBuffer.clearColorImage(**mImages[i], vk::ImageLayout::eTransferDstOptimal,
                vk::ClearColorValue{0.0f, 0.0f, 0.0f, 0.0f}, {
                        vk::ImageSubresourceRange{
                                vk::ImageAspectFlagBits::eColor,
                                0, 1,
                                0, 1
                        }
                });
    }

    BEGIN_DEBUG_LABEL(commandBuffer, "Downsample", glm::vec3(0.8f, 0.8f, 1.0f));
    for (int i = 1; i < mImagesCount; i++) {
        vk::RenderPassBeginInfo renderPassBeginInfo(
                **mRenderPass,
                **mFrameBuffers[i],
                {{0,                 0},
                 {mExtents[i].width, mExtents[i].height}},
                0, nullptr
        );
        commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **mDownsamplePipeline);

        vk::Extent3D extent = mExtents[i];

        vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height));
        vk::Rect2D scissor({0, 0}, {extent.width, extent.height});

        commandBuffer.setViewport(0, {viewport});
        commandBuffer.setScissor(0, {scissor});

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **mPipelineLayout, 0,
                {mLinearDescriptors[i - 1]}, {});

        commandBuffer.draw(3, 1, 0, 0);

        commandBuffer.endRenderPass();
    }
    END_DEBUG_LABEL(commandBuffer);

    BEGIN_DEBUG_LABEL(commandBuffer, "Upsample", glm::vec3(0.8f, 1.0f, 0.8f));
    for (int i = mImagesCount - 2; i >= 0; i--) {
        vk::RenderPassBeginInfo renderPassBeginInfo(
                **mRenderPass,
                **mFrameBuffers[i],
                {{0,                 0},
                 {mExtents[i].width, mExtents[i].height}},
                0, nullptr
        );
        commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **mUpsamplePipeline);

        vk::Extent3D extent = mExtents[i];

        vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height));
        vk::Rect2D scissor({0, 0}, {extent.width, extent.height});

        commandBuffer.setViewport(0, {viewport});
        commandBuffer.setScissor(0, {scissor});

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **mPipelineLayout, 0,
                {mNearestDescriptors[i + 1]}, {});

        commandBuffer.draw(3, 1, 0, 0);

        commandBuffer.endRenderPass();
    }
    END_DEBUG_LABEL(commandBuffer);

    END_DEBUG_LABEL(commandBuffer);
}

void Bloom::createImages() {
    uint32_t width = mWidth;
    uint32_t height = mHeight;

    mImagesCount = VulkanUtils::calculateNumberOfMipLevels({width, height, 1});
    mImagesCount -= 3;

    for (uint32_t i = 0; i < mImagesCount; i++) {
        vk::Extent3D extent{width, height, 1};
        mExtents.push_back(extent);

        width /= 2;
        height /= 2;

        uint32_t queueFamilyIndex = mContext.get().getMainQueueFamilyIndex();

        vk::ImageCreateInfo imageCreateInfo(
                {},
                vk::ImageType::e2D,
                getFormat(),
                extent,
                1,
                1,
                vk::SampleCountFlagBits::e1,
                vk::ImageTiling::eOptimal,
                {vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment
                 | vk::ImageUsageFlagBits::eTransferSrc},
                vk::SharingMode::eExclusive,
                1,
                &queueFamilyIndex,
                vk::ImageLayout::eUndefined
        );
        mImages.push_back(std::make_unique<AllocatedImage>(mContext, imageCreateInfo));
#if defined(VULKAN_DEBUG)
        std::string imageName = "bloom_(1/" + std::to_string(static_cast<uint32_t>(std::pow(2, i))) + ")";
        mContext.get().getDevice().setDebugUtilsObjectNameEXT({
                vk::ObjectType::eImage,
                reinterpret_cast<uint64_t>(**mImages.back()),
                (imageName + "_Image").c_str()
        });
#endif

        vk::ImageViewCreateInfo imageViewCreateInfo(
                {},
                **mImages.back(),
                vk::ImageViewType::e2D,
                getFormat(),
                {},
                vk::ImageSubresourceRange{
                        vk::ImageAspectFlagBits::eColor,
                        0,
                        1,
                        0,
                        1
                }
        );
        mImageViews.push_back(std::make_unique<vk::raii::ImageView>(mContext.get().getDevice(), imageViewCreateInfo));
#if defined(VULKAN_DEBUG)
        mContext.get().getDevice().setDebugUtilsObjectNameEXT({
                vk::ObjectType::eImageView,
                reinterpret_cast<uint64_t>(static_cast<VkImageView>(**mImageViews.back())),
                (imageName + "_ImageView").c_str()
        });
#endif
    }
}

void Bloom::createRenderPasses() {
    vk::AttachmentDescription attachmentDescription(
            {},
            getFormat(),
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eShaderReadOnlyOptimal
    );
    std::array<vk::AttachmentDescription, 1> attachments{attachmentDescription};

    vk::AttachmentReference attachmentReference(
            0,
            vk::ImageLayout::eColorAttachmentOptimal
    );
    std::array<vk::AttachmentReference, 1> attachmentReferences{attachmentReference};

    vk::SubpassDescription subpass(
            {},
            vk::PipelineBindPoint::eGraphics,
            {},
            attachmentReferences,
            {},
            nullptr,
            {}
    );
    std::array<vk::SubpassDescription, 1> subpasses{subpass};

    vk::SubpassDependency inDependency(
            VK_SUBPASS_EXTERNAL,
            0,
            {vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eTransfer},
            {vk::PipelineStageFlagBits::eFragmentShader},
            {vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eTransferWrite},
            {vk::AccessFlagBits::eColorAttachmentWrite}
    );
    vk::SubpassDependency outDependency(
            0,
            VK_SUBPASS_EXTERNAL,
            {vk::PipelineStageFlagBits::eColorAttachmentOutput},
            {vk::PipelineStageFlagBits::eTopOfPipe},
            {vk::AccessFlagBits::eColorAttachmentWrite},
            {vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentWrite}
    );
    std::array<vk::SubpassDependency, 2> dependencies{inDependency, outDependency};

    vk::RenderPassCreateInfo renderPassCreateInfo(
            {},
            attachments,
            subpasses,
            dependencies
    );
    mRenderPass = std::make_unique<vk::raii::RenderPass>(mContext.get().getDevice(), renderPassCreateInfo);
}

void Bloom::createFramebuffers() {
    for (uint32_t i = 0; i < mImagesCount; i++) {
        vk::Extent3D extent = mExtents[i];

        std::array<vk::ImageView, 1> attachments{**mImageViews[i]};
        vk::FramebufferCreateInfo framebufferCreateInfo(
                {},
                **mRenderPass,
                attachments,
                extent.width,
                extent.height,
                1
        );

        mFrameBuffers.emplace_back(std::make_unique<vk::raii::Framebuffer>(mContext.get().getDevice(),
                framebufferCreateInfo));
    }
}

void Bloom::createTextureDescriptors() {
    vk::SamplerCreateInfo samplerCreateInfo(
            {},
            vk::Filter::eLinear,
            vk::Filter::eLinear,
            vk::SamplerMipmapMode::eNearest,
            vk::SamplerAddressMode::eClampToEdge,
            vk::SamplerAddressMode::eClampToEdge,
            vk::SamplerAddressMode::eClampToEdge,
            0.0f,
            false,
            1.0f,
            false,
            vk::CompareOp::eNever,
            0.0f,
            0.0f,
            vk::BorderColor::eFloatOpaqueWhite,
            false
    );
    mLinearSampler = std::make_unique<vk::raii::Sampler>(mContext.get().getDevice(), samplerCreateInfo);

//    samplerCreateInfo.minFilter = vk::Filter::eNearest;
//    samplerCreateInfo.magFilter = vk::Filter::eNearest;
    mNearestSampler = std::make_unique<vk::raii::Sampler>(mContext.get().getDevice(), samplerCreateInfo);

    vk::DescriptorSetLayoutBinding textureLayoutBinding(
            0,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            {vk::ShaderStageFlagBits::eFragment},
            nullptr//&**mLinearSampler
    );

    mTextureDescriptorGenerator = std::make_unique<DescriptorGenerator>(mContext.get(),
            std::vector<vk::DescriptorSetLayoutBinding>{textureLayoutBinding}, mImagesCount * 2);

    {
        mLinearDescriptors = mTextureDescriptorGenerator->createDescriptorSets(mImagesCount);

        std::vector<vk::DescriptorImageInfo> imageInfos;
        std::vector<vk::WriteDescriptorSet> descriptorWrites;
        imageInfos.reserve(mImagesCount);
        descriptorWrites.reserve(mImagesCount);

        for (int i = 0; i < mImagesCount; i++) {
            vk::ImageView imageView = (i == 0) ? *mFrameRenderer.get().getMainColorImageView() : **mImageViews[i];

            imageInfos.emplace_back(
                    **mLinearSampler,
                    imageView,
                    vk::ImageLayout::eShaderReadOnlyOptimal
            );

            descriptorWrites.emplace_back(
                    mLinearDescriptors[i],
                    0,
                    0,
                    1,
                    vk::DescriptorType::eCombinedImageSampler,
                    &imageInfos.back(),
                    nullptr,
                    nullptr
            );
        }

        mContext.get().getDevice().updateDescriptorSets(descriptorWrites, {});
    }

    {
        mNearestDescriptors = mTextureDescriptorGenerator->createDescriptorSets(mImagesCount);

        std::vector<vk::DescriptorImageInfo> imageInfos;
        std::vector<vk::WriteDescriptorSet> descriptorWrites;
        imageInfos.reserve(mImagesCount);
        descriptorWrites.reserve(mImagesCount);

        for (int i = 0; i < mImagesCount; i++) {
            vk::ImageView imageView = (i == 0) ? *mFrameRenderer.get().getMainColorImageView() : **mImageViews[i];

            imageInfos.emplace_back(
                    **mNearestSampler,
                    imageView,
                    vk::ImageLayout::eShaderReadOnlyOptimal
            );

            descriptorWrites.emplace_back(
                    mNearestDescriptors[i],
                    0,
                    0,
                    1,
                    vk::DescriptorType::eCombinedImageSampler,
                    &imageInfos.back(),
                    nullptr,
                    nullptr
            );
        }

        mContext.get().getDevice().updateDescriptorSets(descriptorWrites, {});
    }
}

void Bloom::createPipelines() {
    vk::raii::ShaderModule vertexShaderModule = VulkanUtils::createShaderModule(mContext,
            *AssetFactory::getShaderAsset("post_processing/fullscreen_triangle.vert"));
    vk::raii::ShaderModule downsampleShaderModule = VulkanUtils::createShaderModule(mContext,
            *AssetFactory::getShaderAsset("post_processing/bloom_downsample.frag"));
    vk::raii::ShaderModule upsampleShaderModule = VulkanUtils::createShaderModule(mContext,
            *AssetFactory::getShaderAsset("post_processing/bloom_upsample.frag"));

    vk::PipelineShaderStageCreateInfo vertexShaderStage(
            {},
            vk::ShaderStageFlagBits::eVertex,
            *vertexShaderModule,
            "main",
            nullptr
    );
    vk::PipelineShaderStageCreateInfo downSampleShaderStage(
            {},
            vk::ShaderStageFlagBits::eFragment,
            *downsampleShaderModule,
            "main",
            nullptr
    );
    vk::PipelineShaderStageCreateInfo upSampleShaderStage(
            {},
            vk::ShaderStageFlagBits::eFragment,
            *upsampleShaderModule,
            "main",
            nullptr
    );

    std::array<vk::PipelineShaderStageCreateInfo, 2> downSampleStages = {vertexShaderStage, downSampleShaderStage};
    std::array<vk::PipelineShaderStageCreateInfo, 2> upSampleStages = {vertexShaderStage, upSampleShaderStage};

    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo(
            {},
            vk::PrimitiveTopology::eTriangleList,
            false
    );

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo(
            {},
            1, nullptr,
            1, nullptr
    );

    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(
            {},
            false,
            false,
            vk::PolygonMode::eFill,
            vk::CullModeFlagBits::eNone,
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
            false,
            false,
            vk::CompareOp::eAlways,
            false,
            false,
            {},
            {},
            0.0f,
            1.0f
    );

    vk::PipelineColorBlendAttachmentState colorBlendAttachment(
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
    std::array<vk::PipelineColorBlendAttachmentState, 1> colorBlendAttachments{colorBlendAttachment};

    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo(
            {},
            false,
            vk::LogicOp::eCopy,
            colorBlendAttachments,
            {0.0f, 0.0f, 0.0f, 0.0f}
    );

    std::array<vk::DynamicState, 2> dynamicStates{vk::DynamicState::eViewport, vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo(
            {},
            dynamicStates
    );

    std::array<vk::DescriptorSetLayout, 1> descriptorSetLayouts{
            *mTextureDescriptorGenerator->getLayout()
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            {},
            descriptorSetLayouts,
            {}
    );

    mPipelineLayout = std::make_unique<vk::raii::PipelineLayout>(mContext.get().getDevice(), pipelineLayoutCreateInfo);

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo(
            {},
            downSampleStages,
            &vertexInputStateCreateInfo,
            &inputAssemblyStateCreateInfo,
            nullptr,
            &viewportStateCreateInfo,
            &rasterizationStateCreateInfo,
            &multisampleStateCreateInfo,
            &depthStencilStateCreateInfo,
            &colorBlendStateCreateInfo,
            &dynamicStateCreateInfo,
            **mPipelineLayout,
            **mRenderPass,
            0,
            nullptr,
            0
    );
    mDownsamplePipeline = std::make_unique<vk::raii::Pipeline>(mContext.get().getDevice(), nullptr, pipelineCreateInfo);

    pipelineCreateInfo.setStages(upSampleStages);

    vk::PipelineColorBlendAttachmentState upsampleColorBlendAttachment(
            true,
            vk::BlendFactor::eOne,
            vk::BlendFactor::eOne,
            vk::BlendOp::eAdd,
            vk::BlendFactor::eOne,
            vk::BlendFactor::eOne,
            vk::BlendOp::eAdd,
            vk::ColorComponentFlags{vk::ColorComponentFlagBits::eR |
                                    vk::ColorComponentFlagBits::eG |
                                    vk::ColorComponentFlagBits::eB |
                                    vk::ColorComponentFlagBits::eA}
    );
    std::array<vk::PipelineColorBlendAttachmentState, 1> upsampleColorBlendAttachments{upsampleColorBlendAttachment};

    vk::PipelineColorBlendStateCreateInfo upsampleColorBlendStateCreateInfo(
            {},
            false,
            vk::LogicOp::eCopy,
            upsampleColorBlendAttachments,
            {0.0f, 0.0f, 0.0f, 0.0f}
    );
    pipelineCreateInfo.setPColorBlendState(&upsampleColorBlendStateCreateInfo);
    mUpsamplePipeline = std::make_unique<vk::raii::Pipeline>(mContext.get().getDevice(), nullptr, pipelineCreateInfo);
}
