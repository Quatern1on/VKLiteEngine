#include "pch.h"
#include "ToneMapping.h"

ToneMapping::ToneMapping(const VulkanContext& context, const FrameRenderer& frameRenderer)
        : mContext(context), mFrameRenderer(frameRenderer) {
    createImage();
    createRenderPass();
    createFramebuffer();
    createTextureDescriptor();
    createPipeline();
}

void ToneMapping::render(vk::raii::CommandBuffer& commandBuffer) {
    uint32_t width = mFrameRenderer.get().getWidth();
    uint32_t height = mFrameRenderer.get().getHeight();

    BEGIN_DEBUG_LABEL(commandBuffer, "Tonemapping", glm::vec3(1.0f, 1.0f, 1.0f));

    vk::RenderPassBeginInfo renderPassBeginInfo(
            **mRenderPass,
            **mFrameBuffer,
            {{0,     0},
             {width, height}},
            0, nullptr
    );
    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **mPipeline);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **mPipelineLayout, 0,
            {mTexturesSet}, {});

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();

    END_DEBUG_LABEL(commandBuffer);
}

void ToneMapping::createImage() {
    vk::Format format = Engine::getInstance().getSwapChain().getFormat();

    uint32_t width = mFrameRenderer.get().getWidth();
    uint32_t height = mFrameRenderer.get().getHeight();

    vk::Extent3D extent{width, height, 1};

    uint32_t queueFamilyIndex = mContext.get().getMainQueueFamilyIndex();

    vk::ImageCreateInfo imageCreateInfo(
            {},
            vk::ImageType::e2D,
            format,
            extent,
            1,
            1,
            vk::SampleCountFlagBits::e1,
            vk::ImageTiling::eOptimal,
            {vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc},
            vk::SharingMode::eExclusive,
            1,
            &queueFamilyIndex,
            vk::ImageLayout::eUndefined
    );
    mImage = std::make_unique<AllocatedImage>(mContext, imageCreateInfo);
#if defined(VULKAN_DEBUG)
    mContext.get().getDevice().setDebugUtilsObjectNameEXT({
            vk::ObjectType::eImage,
            reinterpret_cast<uint64_t>(**mImage),
            "post_tonemap_Image"
    });
#endif

    vk::ImageViewCreateInfo imageViewCreateInfo(
            {},
            **mImage,
            vk::ImageViewType::e2D,
            format,
            {},
            vk::ImageSubresourceRange{
                    vk::ImageAspectFlagBits::eColor,
                    0,
                    1,
                    0,
                    1
            }
    );
    mImageView = std::make_unique<vk::raii::ImageView>(mContext.get().getDevice(), imageViewCreateInfo);
#if defined(VULKAN_DEBUG)
    mContext.get().getDevice().setDebugUtilsObjectNameEXT({
            vk::ObjectType::eImageView,
            reinterpret_cast<uint64_t>(static_cast<VkImageView>(**mImageView)),
            "post_tonemap_Image"
    });
#endif
}

void ToneMapping::createRenderPass() {
    vk::AttachmentDescription attachmentDescription(
            {},
            Engine::getInstance().getSwapChain().getFormat(),
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferSrcOptimal
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

    vk::RenderPassCreateInfo renderPassCreateInfo(
            {},
            attachments,
            subpasses,
            {}
    );
    mRenderPass = std::make_unique<vk::raii::RenderPass>(mContext.get().getDevice(), renderPassCreateInfo);
}

void ToneMapping::createFramebuffer() {
    uint32_t width = mFrameRenderer.get().getWidth();
    uint32_t height = mFrameRenderer.get().getHeight();

    std::array<vk::ImageView, 1> attachments{**mImageView};
    vk::FramebufferCreateInfo framebufferCreateInfo(
            {},
            **mRenderPass,
            attachments,
            width,
            height,
            1
    );

    mFrameBuffer = std::make_unique<vk::raii::Framebuffer>(mContext.get().getDevice(),
            framebufferCreateInfo);
}

void ToneMapping::createTextureDescriptor() {
    vk::SamplerCreateInfo samplerCreateInfo(
            {},
            vk::Filter::eNearest,
            vk::Filter::eNearest,
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
            vk::BorderColor::eFloatTransparentBlack,
            false
    );
    mTextureSampler = std::make_unique<vk::raii::Sampler>(mContext.get().getDevice(), samplerCreateInfo);

    vk::DescriptorSetLayoutBinding HDRLayoutBinding(
            0,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            {vk::ShaderStageFlagBits::eFragment},
            &**mTextureSampler
    );
    vk::DescriptorSetLayoutBinding bloomLayoutBinding(
            1,
            vk::DescriptorType::eCombinedImageSampler,
            1,
            {vk::ShaderStageFlagBits::eFragment},
            &**mTextureSampler
    );

    mTexturesDescriptorGenerator = std::make_unique<DescriptorGenerator>(mContext.get(),
            std::vector<vk::DescriptorSetLayoutBinding>{HDRLayoutBinding, bloomLayoutBinding}, 1);

    mTexturesSet = mTexturesDescriptorGenerator->createDescriptorSet();

    std::vector<vk::WriteDescriptorSet> descriptorWrites;
    descriptorWrites.reserve(2);

    vk::DescriptorImageInfo HDRInfo(
            vk::Sampler(nullptr),
            *mFrameRenderer.get().getMainColorImageView(),
            vk::ImageLayout::eShaderReadOnlyOptimal
    );
    vk::DescriptorImageInfo bloomInfo(
            vk::Sampler(nullptr),
            *mFrameRenderer.get().getBloom().getImageView(),
            vk::ImageLayout::eShaderReadOnlyOptimal
    );

    descriptorWrites.emplace_back(
            mTexturesSet,
            0,
            0,
            1,
            vk::DescriptorType::eCombinedImageSampler,
            &HDRInfo,
            nullptr,
            nullptr
    );
    descriptorWrites.emplace_back(
            mTexturesSet,
            1,
            0,
            1,
            vk::DescriptorType::eCombinedImageSampler,
            &bloomInfo,
            nullptr,
            nullptr
    );

    mContext.get().getDevice().updateDescriptorSets(descriptorWrites, {});
}

void ToneMapping::createPipeline() {
    uint32_t width = mFrameRenderer.get().getWidth();
    uint32_t height = mFrameRenderer.get().getHeight();

    vk::raii::ShaderModule vertexShaderModule = VulkanUtils::createShaderModule(mContext,
            *AssetFactory::getShaderAsset("post_processing/fullscreen_triangle.vert"));
    vk::raii::ShaderModule tonemappingShaderModule = VulkanUtils::createShaderModule(mContext,
            *AssetFactory::getShaderAsset("post_processing/aces.frag"));

    vk::PipelineShaderStageCreateInfo vertexShaderStage(
            {},
            vk::ShaderStageFlagBits::eVertex,
            *vertexShaderModule,
            "main",
            nullptr
    );
    vk::PipelineShaderStageCreateInfo tonemappingShaderStage(
            {},
            vk::ShaderStageFlagBits::eFragment,
            *tonemappingShaderModule,
            "main",
            nullptr
    );

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vertexShaderStage, tonemappingShaderStage};

    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo(
            {},
            vk::PrimitiveTopology::eTriangleList,
            false
    );

    vk::Viewport viewport(
            0.0f,
            0.0f,
            static_cast<float>(width),
            static_cast<float>(height)
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


    std::array<vk::DescriptorSetLayout, 1> descriptorSetLayouts{
            *mTexturesDescriptorGenerator->getLayout()
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            {},
            descriptorSetLayouts,
            {}
    );

    mPipelineLayout = std::make_unique<vk::raii::PipelineLayout>(mContext.get().getDevice(), pipelineLayoutCreateInfo);

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo(
            {},
            shaderStages,
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
            **mRenderPass,
            0,
            nullptr,
            0
    );
    mPipeline = std::make_unique<vk::raii::Pipeline>(mContext.get().getDevice(), nullptr, pipelineCreateInfo);

}
