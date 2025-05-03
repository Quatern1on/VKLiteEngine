#include "pch.h"
#include "DeferredShading.h"

#include "Scene/Mesh/Mesh.h"
#include "Scene/Camera/CameraComponent.h"
#include "Scene/Mesh/MeshComponent.h"

DeferredShading::DeferredShading(const VulkanContext& context, const FrameRenderer& frameRenderer, uint32_t width,
        uint32_t height)
        : mContext(std::ref(context)), mFrameRenderer(std::ref(frameRenderer)) {
    mGBuffer = std::make_unique<GBuffer>(context, kGBufferImageDescriptions, width, height);

    createRenderPass();
    createFramebuffer();
    createClearValues();

    mGeometryPass = std::make_unique<GeometryPass>(context, *this);
    mLightPass = std::make_unique<LightPass>(context, *this, frameRenderer);
}

void DeferredShading::render(Scene& scene, vk::raii::CommandBuffer& commandBuffer) {
    BEGIN_DEBUG_LABEL(commandBuffer, "Deferred shading", glm::vec3(1.0f, 1.0f, 1.0f));

    std::vector<std::reference_wrapper<MeshComponent>> meshes = scene.getRoot().getComponentsRecursive<MeshComponent>();

    vk::RenderPassBeginInfo renderPassBeginInfo(
            **mRenderPass,
            **mFrameBuffer,
            {{0,                    0},
             {mGBuffer->getWidth(), mGBuffer->getHeight()}},
            mClearValues
    );
    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    mGeometryPass->render(commandBuffer, meshes, *scene.getCamera());

    commandBuffer.nextSubpass(vk::SubpassContents::eInline);

    mLightPass->render(commandBuffer, scene);

    commandBuffer.endRenderPass();

    END_DEBUG_LABEL(commandBuffer);
}

void DeferredShading::createRenderPass() {
    std::vector<vk::AttachmentDescription> attachmentDescriptions;
    std::vector<vk::AttachmentReference> geometryColorAttachments;
    std::vector<vk::AttachmentReference> lightColorAttachments;
    std::vector<vk::AttachmentReference> lightInputAttachments;
    vk::AttachmentReference depthAttachmentReference;
    vk::AttachmentReference depthAttachmentReadOnlyReference;

    std::size_t attachmentCount = mGBuffer->size() + 2;

    attachmentDescriptions.reserve(attachmentCount);
    geometryColorAttachments.reserve(attachmentCount - 1);
    lightColorAttachments.reserve(1);
    lightColorAttachments.reserve(attachmentCount - 2);

    //Main image color
    attachmentDescriptions.emplace_back(
            vk::AttachmentDescriptionFlags{},
            mFrameRenderer.get().getMainColorImageFormat(),
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            //TODO revert back to vk::ImageLayout::eShaderReadOnlyOptimal
            vk::ImageLayout::eShaderReadOnlyOptimal
    );
    geometryColorAttachments.emplace_back(
            0,
            vk::ImageLayout::eColorAttachmentOptimal
    );
    lightColorAttachments.emplace_back(
            0,
            vk::ImageLayout::eColorAttachmentOptimal
    );

    //Main image depth
    attachmentDescriptions.emplace_back(
            vk::AttachmentDescriptionFlags{},
            mFrameRenderer.get().getMainDepthImageFormat(),
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            //TODO revert back to vk::ImageLayout::eShaderReadOnlyOptimal
            vk::ImageLayout::eTransferSrcOptimal
    );
    depthAttachmentReference = {
            1,
            vk::ImageLayout::eDepthStencilAttachmentOptimal
    };
    depthAttachmentReadOnlyReference = {
            1,
            vk::ImageLayout::eDepthStencilReadOnlyOptimal
    };

    for (const auto& gBufferImageDescription : kGBufferImageDescriptions) {
        attachmentDescriptions.emplace_back(
                vk::AttachmentDescriptionFlags{},
                gBufferImageDescription.format,
                vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eStore,
                vk::AttachmentLoadOp::eDontCare,
                vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined,
                //TODO revert back to vk::ImageLayout::eShaderReadOnlyOptimal
                vk::ImageLayout::eShaderReadOnlyOptimal
        );
        geometryColorAttachments.emplace_back(
                attachmentDescriptions.size() - 1,
                vk::ImageLayout::eColorAttachmentOptimal
        );
        lightInputAttachments.emplace_back(
                attachmentDescriptions.size() - 1,
                vk::ImageLayout::eShaderReadOnlyOptimal
        );
    }

    lightInputAttachments.emplace_back(
            1,
            vk::ImageLayout::eDepthStencilAttachmentOptimal
    );

    vk::SubpassDescription geometrySubpass(
            {},
            vk::PipelineBindPoint::eGraphics,
            {},
            geometryColorAttachments,
            {},
            &depthAttachmentReference,
            {}
    );

    vk::SubpassDescription lightSubpass(
            {},
            vk::PipelineBindPoint::eGraphics,
            lightInputAttachments,
            lightColorAttachments,
            {},
            &depthAttachmentReadOnlyReference,
            {}
    );

    std::array<vk::SubpassDescription, 2> subpasses{geometrySubpass, lightSubpass};

    vk::SubpassDependency dependency(
            0,
            1,
            vk::PipelineStageFlagBits::eAllGraphics,
            vk::PipelineStageFlagBits::eFragmentShader,
            {vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite},
            vk::AccessFlagBits::eInputAttachmentRead,
            {}
    );
    vk::SubpassDependency outputDependency(
            1,
            VK_SUBPASS_EXTERNAL,
            vk::PipelineStageFlagBits::eAllGraphics,
            vk::PipelineStageFlagBits::eFragmentShader,
            {vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite},
            vk::AccessFlagBits::eShaderRead,
            {}
    );
    std::array<vk::SubpassDependency, 2> dependencies{dependency, outputDependency};

    vk::RenderPassCreateInfo renderPassCreateInfo(
            {},
            attachmentDescriptions,
            subpasses,
            dependencies
    );

    mRenderPass = std::make_unique<vk::raii::RenderPass>(mContext.get().getDevice(), renderPassCreateInfo);
}

void DeferredShading::createFramebuffer() {
    std::vector<vk::ImageView> imageViews;
    imageViews.reserve(mGBuffer->size() + 2);

    imageViews.push_back(*mFrameRenderer.get().getMainColorImageView());
    imageViews.push_back(*mFrameRenderer.get().getMainDepthImageView());
    for (const auto& raiiImageView : mGBuffer->getImageViews()) {
        imageViews.push_back(**raiiImageView);
    }

    vk::FramebufferCreateInfo framebufferCreateInfo(
            {},
            **mRenderPass,
            imageViews,
            mGBuffer->getWidth(),
            mGBuffer->getHeight(),
            1
    );

    mFrameBuffer = std::make_unique<vk::raii::Framebuffer>(mContext.get().getDevice(), framebufferCreateInfo);
}

void DeferredShading::createClearValues() {
    mClearValues.reserve(mGBuffer->size() + 2);

    //Main image color
    mClearValues.emplace_back(
            vk::ClearColorValue{0.0, 0.0f, 0.0f, 1.0f}
    );

    //Main image depth
    mClearValues.emplace_back(
            vk::ClearDepthStencilValue{1.0f, 0U}
    );

    for (int i = 0; i < mGBuffer->size(); i++) {
        mClearValues.emplace_back(
                vk::ClearColorValue{0.0, 0.0f, 0.0f, 1.0f}
        );
    }
}