#include "pch.h"
#include "VulkanContext.h"

#include "Core/Engine.h"
#include "Core/Utils.h"
#include "Vulkan/ValidationLayersMessenger.h"

VulkanContext::VulkanContext(const Window& window)
        : mMainQueueFamilyIndex(0), mWindow(std::ref(window)) {
    mContext = std::make_unique<vk::raii::Context>();
    createInstance();

#ifdef ENABLE_VALIDATION_LAYERS
    createDebugMessenger();
#endif

    mSurface = window.createSurface(*mInstance);
    selectPhysicalDevice();
    createDevice();
    retrieveMainQueue();
    mAllocator = std::make_unique<Allocator>(*mInstance, *mPhysicalDevice, *mDevice);
    createSyncOperationsObjects();
}

void VulkanContext::executeCommandBufferSync(
        std::function<void(const vk::raii::CommandBuffer& cmd)>&& recordBuffer) const {
    mSyncOperationsCommandPool->reset();

    static vk::CommandBufferBeginInfo beginInfo(
            {vk::CommandBufferUsageFlagBits::eOneTimeSubmit},
            nullptr
    );
    mSyncOperationsCommandBuffer->begin(beginInfo);
    recordBuffer(*mSyncOperationsCommandBuffer);
    mSyncOperationsCommandBuffer->end();

    vk::SubmitInfo submitInfo(
            0, nullptr,
            {},
            1, &**mSyncOperationsCommandBuffer,
            0, nullptr
    );
    mMainQueue->submit({submitInfo}, **mSyncOperationsFence);

    using namespace std::chrono_literals;
    std::chrono::nanoseconds timeout = 10s;
    vk::Result result = mDevice->waitForFences(**mSyncOperationsFence, true, timeout.count());

    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Error while waiting for a sync operations fence");
    }
    getDevice().resetFences({**mSyncOperationsFence});
}

void VulkanContext::createInstance() {
    vk::ApplicationInfo applicationInfo(
            "Vulkan PBR",
            VK_MAKE_VERSION(1, 0, 0),
            "Custom engine",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_3
    );

    std::vector<std::string> requiredLayers = getRequiredLayers();
    std::vector<std::string> requiredExtensions = getRequiredInstanceExtensions();

    if (!checkLayerSupport(requiredLayers)) {
        throw std::runtime_error("Some of the required validation layers are not supported on this system");
    }
    if (!checkInstanceExtensionSupport(requiredExtensions)) {
        throw std::runtime_error("Some of the required mInstance extensions are not supported on this system");
    }

    std::vector<const char*> requiredLayersCStr(Utils::toCStrVector(requiredLayers));
    std::vector<const char*> requiredExtensionsCStr(Utils::toCStrVector(requiredExtensions));

    vk::InstanceCreateInfo instanceCreateInfo(
            {},
            &applicationInfo,
            requiredLayersCStr,
            requiredExtensionsCStr
    );

    vk::DebugUtilsMessengerCreateInfoEXT instanceMessengerCreateInfo(
            {},
            {
//                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
//                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
            },
            {
//                    vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding
            },
            &ValidationLayersMessenger::debugCallback
    );

    vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> instanceCreateChain = {
            instanceCreateInfo,
            instanceMessengerCreateInfo
    };

#ifndef ENABLE_VALIDATION_LAYERS
    instanceCreateChain.unlink<vk::DebugUtilsMessengerCreateInfoEXT>();
#endif

    mInstance = std::make_unique<vk::raii::Instance>(*mContext, instanceCreateChain.get<vk::InstanceCreateInfo>());

    mEnabledLayers = requiredLayers;
    mEnabledInstanceExtensions = requiredExtensions;
}

void VulkanContext::createDebugMessenger() {
    vk::DebugUtilsMessengerCreateInfoEXT globalMessengerCreateInfo(
            {},
            {
//                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
//                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
            },
            {
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding
            },
            &ValidationLayersMessenger::debugCallback
    );
    mDebugMessenger = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(*mInstance, globalMessengerCreateInfo);
}

void VulkanContext::selectPhysicalDevice() {
    vk::raii::PhysicalDevices physicalDevices(*mInstance);

    //Filter unsupported devices
    for (int i = 0; i < physicalDevices.size(); i++) {
        const vk::raii::PhysicalDevice& physicalDevice = physicalDevices[i];
        if (!checkDeviceSupport(physicalDevice)) {
            physicalDevices.erase(physicalDevices.cbegin() + i);
            i--;
        }
    }

    if (physicalDevices.empty()) {
        throw std::runtime_error("No supported physical device available");
    }

    //Prefer discrete GPU
    std::sort(physicalDevices.begin(), physicalDevices.end(),
            [](const vk::raii::PhysicalDevice& a, const vk::raii::PhysicalDevice& b) -> bool {
                return a.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
            }
    );

    mPhysicalDevice = std::make_unique<vk::raii::PhysicalDevice>(std::move(physicalDevices[0]));

    LOG(INFO) << "Selected physical device: " << mPhysicalDevice->getProperties().deviceName;

    mPhysicalDeviceProperties = mPhysicalDevice->getProperties();
}

void VulkanContext::createDevice() {
    float queuePriority = 1.0f;

    mMainQueueFamilyIndex = selectMainQueue(*mPhysicalDevice).value();

    vk::DeviceQueueCreateInfo mainQueueCreateInfo(
            {},
            mMainQueueFamilyIndex,
            1,
            &queuePriority
    );

    std::vector<std::string> requiredExtensions = getRequiredDeviceExtensions();
    vk::PhysicalDeviceFeatures requiredDeviceFeatures = getRequiredDeviceFeatures();

    std::vector<const char*> requiredLayersCStr(Utils::toCStrVector(mEnabledLayers));
    std::vector<const char*> requiredExtensionsCStr(Utils::toCStrVector(requiredExtensions));

    vk::DeviceCreateInfo deviceCreateInfo(
            {},
            mainQueueCreateInfo,
            requiredLayersCStr,
            requiredExtensionsCStr,
            &requiredDeviceFeatures
    );

    mDevice = std::make_unique<vk::raii::Device>(*mPhysicalDevice, deviceCreateInfo);

    mEnabledDeviceExtensions = requiredExtensions;
    mEnabledDeviceFeatures = requiredDeviceFeatures;
}

void VulkanContext::retrieveMainQueue() {
    mMainQueue = std::make_unique<vk::raii::Queue>(mDevice->getQueue(mMainQueueFamilyIndex, 0));
}

void VulkanContext::createSyncOperationsObjects() {
    vk::CommandPoolCreateInfo commandPoolCreateInfo(
            {},
            mMainQueueFamilyIndex
    );
    mSyncOperationsCommandPool = std::make_unique<vk::raii::CommandPool>(*mDevice, commandPoolCreateInfo);

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo(
            **mSyncOperationsCommandPool,
            vk::CommandBufferLevel::ePrimary,
            1
    );
    mSyncOperationsCommandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(
            mDevice->allocateCommandBuffers(commandBufferAllocateInfo)[0]));

    mSyncOperationsFence = std::make_unique<vk::raii::Fence>(std::move(
            VulkanUtils::createFence(*this, false)
    ));
}

std::vector<std::string> VulkanContext::getRequiredLayers() const {
#ifdef ENABLE_VALIDATION_LAYERS
    return {"VK_LAYER_KHRONOS_validation"};
#else
    return {};
#endif
}

std::vector<std::string> VulkanContext::getRequiredInstanceExtensions() const {
    std::vector<std::string> extensions = mWindow.get().getRequiredInstanceExtensions();
#if defined(ENABLE_VALIDATION_LAYERS) || defined(VULKAN_DEBUG)
    extensions.emplace_back("VK_EXT_debug_utils");
#endif
    return extensions;
}

std::vector<std::string> VulkanContext::getRequiredDeviceExtensions() const {
    return {"VK_KHR_swapchain"};
}

vk::PhysicalDeviceFeatures VulkanContext::getRequiredDeviceFeatures() const {
    vk::PhysicalDeviceFeatures requiredFeatures{};

    requiredFeatures.samplerAnisotropy = true;

    return requiredFeatures;
}

bool VulkanContext::checkLayerSupport(const std::vector<std::string>& layers) {
    std::vector<vk::LayerProperties> availableLayers = mContext->enumerateInstanceLayerProperties();

    for (const std::string& requiredLayerName : layers) {
        bool layerSupported = false;
        for (const vk::LayerProperties& availableLayer : availableLayers) {
            if (requiredLayerName == availableLayer.layerName) {
                layerSupported = true;
                break;
            }
        }

        if (!layerSupported) {
            return false;
        }
    }

    return true;
}

bool VulkanContext::checkInstanceExtensionSupport(const std::vector<std::string>& extensions) {
    std::vector<vk::ExtensionProperties> availableExtensions = mContext->enumerateInstanceExtensionProperties();

    for (const std::string& requiredExtensionName : extensions) {
        bool extensionSupported = false;
        for (const vk::ExtensionProperties& availableExtension : availableExtensions) {
            if (requiredExtensionName == availableExtension.extensionName) {
                extensionSupported = true;
                break;
            }
        }

        if (!extensionSupported) {
            return false;
        }
    }

    return true;
}

bool VulkanContext::checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& device,
        const std::vector<std::string>& extensions) {
    std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

    for (const std::string& requiredExtensionName : extensions) {
        bool extensionSupported = false;
        for (const vk::ExtensionProperties& availableExtension : availableExtensions) {
            if (requiredExtensionName == availableExtension.extensionName) {
                extensionSupported = true;
                break;
            }
        }

        if (!extensionSupported) {
            return false;
        }
    }

    return true;
}

bool VulkanContext::checkDeviceFeaturesSupport(const vk::raii::PhysicalDevice& device,
        const vk::PhysicalDeviceFeatures& features) {
    vk::PhysicalDeviceFeatures availableFeatures = device.getFeatures();

    bool supported = true;

    supported &= availableFeatures.robustBufferAccess >= features.robustBufferAccess;
    supported &= availableFeatures.fullDrawIndexUint32 >= features.fullDrawIndexUint32;
    supported &= availableFeatures.imageCubeArray >= features.imageCubeArray;
    supported &= availableFeatures.independentBlend >= features.independentBlend;
    supported &= availableFeatures.geometryShader >= features.geometryShader;
    supported &= availableFeatures.tessellationShader >= features.tessellationShader;
    supported &= availableFeatures.sampleRateShading >= features.sampleRateShading;
    supported &= availableFeatures.dualSrcBlend >= features.dualSrcBlend;
    supported &= availableFeatures.logicOp >= features.logicOp;
    supported &= availableFeatures.multiDrawIndirect >= features.multiDrawIndirect;
    supported &= availableFeatures.drawIndirectFirstInstance >= features.drawIndirectFirstInstance;
    supported &= availableFeatures.depthClamp >= features.depthClamp;
    supported &= availableFeatures.depthBiasClamp >= features.depthBiasClamp;
    supported &= availableFeatures.fillModeNonSolid >= features.fillModeNonSolid;
    supported &= availableFeatures.depthBounds >= features.depthBounds;
    supported &= availableFeatures.wideLines >= features.wideLines;
    supported &= availableFeatures.largePoints >= features.largePoints;
    supported &= availableFeatures.alphaToOne >= features.alphaToOne;
    supported &= availableFeatures.multiViewport >= features.multiViewport;
    supported &= availableFeatures.samplerAnisotropy >= features.samplerAnisotropy;
    supported &= availableFeatures.textureCompressionETC2 >= features.textureCompressionETC2;
    supported &= availableFeatures.textureCompressionASTC_LDR >= features.textureCompressionASTC_LDR;
    supported &= availableFeatures.textureCompressionBC >= features.textureCompressionBC;
    supported &= availableFeatures.occlusionQueryPrecise >= features.occlusionQueryPrecise;
    supported &= availableFeatures.pipelineStatisticsQuery >= features.pipelineStatisticsQuery;
    supported &= availableFeatures.vertexPipelineStoresAndAtomics >= features.vertexPipelineStoresAndAtomics;
    supported &= availableFeatures.fragmentStoresAndAtomics >= features.fragmentStoresAndAtomics;
    supported &= availableFeatures.shaderTessellationAndGeometryPointSize >=
                 features.shaderTessellationAndGeometryPointSize;
    supported &= availableFeatures.shaderImageGatherExtended >= features.shaderImageGatherExtended;
    supported &= availableFeatures.shaderStorageImageExtendedFormats >= features.shaderStorageImageExtendedFormats;
    supported &= availableFeatures.shaderStorageImageMultisample >= features.shaderStorageImageMultisample;
    supported &= availableFeatures.shaderStorageImageReadWithoutFormat >= features.shaderStorageImageReadWithoutFormat;
    supported &= availableFeatures.shaderStorageImageWriteWithoutFormat >=
                 features.shaderStorageImageWriteWithoutFormat;
    supported &= availableFeatures.shaderUniformBufferArrayDynamicIndexing >=
                 features.shaderUniformBufferArrayDynamicIndexing;
    supported &= availableFeatures.shaderSampledImageArrayDynamicIndexing >=
                 features.shaderSampledImageArrayDynamicIndexing;
    supported &= availableFeatures.shaderStorageBufferArrayDynamicIndexing >=
                 features.shaderStorageBufferArrayDynamicIndexing;
    supported &= availableFeatures.shaderStorageImageArrayDynamicIndexing >=
                 features.shaderStorageImageArrayDynamicIndexing;
    supported &= availableFeatures.shaderClipDistance >= features.shaderClipDistance;
    supported &= availableFeatures.shaderCullDistance >= features.shaderCullDistance;
    supported &= availableFeatures.shaderFloat64 >= features.shaderFloat64;
    supported &= availableFeatures.shaderInt64 >= features.shaderInt64;
    supported &= availableFeatures.shaderInt16 >= features.shaderInt16;
    supported &= availableFeatures.shaderResourceResidency >= features.shaderResourceResidency;
    supported &= availableFeatures.shaderResourceMinLod >= features.shaderResourceMinLod;
    supported &= availableFeatures.sparseBinding >= features.sparseBinding;
    supported &= availableFeatures.sparseResidencyBuffer >= features.sparseResidencyBuffer;
    supported &= availableFeatures.sparseResidencyImage2D >= features.sparseResidencyImage2D;
    supported &= availableFeatures.sparseResidencyImage3D >= features.sparseResidencyImage3D;
    supported &= availableFeatures.sparseResidency2Samples >= features.sparseResidency2Samples;
    supported &= availableFeatures.sparseResidency4Samples >= features.sparseResidency4Samples;
    supported &= availableFeatures.sparseResidency8Samples >= features.sparseResidency8Samples;
    supported &= availableFeatures.sparseResidency16Samples >= features.sparseResidency16Samples;
    supported &= availableFeatures.sparseResidencyAliased >= features.sparseResidencyAliased;
    supported &= availableFeatures.variableMultisampleRate >= features.variableMultisampleRate;
    supported &= availableFeatures.inheritedQueries >= features.inheritedQueries;

    return supported;
}

bool VulkanContext::checkDeviceSupport(const vk::raii::PhysicalDevice& device) {
    bool extensionSupport = checkDeviceExtensionSupport(device, getRequiredDeviceExtensions());
    bool featureSupport = checkDeviceFeaturesSupport(device, getRequiredDeviceFeatures());
    bool queueSupport = selectMainQueue(device).has_value();

    return extensionSupport && featureSupport && queueSupport;
}

std::optional<uint32_t> VulkanContext::selectMainQueue(const vk::raii::PhysicalDevice& device) {
    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    vk::QueueFlags requiredFlags = {
            vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer
    };

    for (uint32_t familyIndex = 0; familyIndex < queueFamilies.size(); familyIndex++) {
        const vk::QueueFamilyProperties queueFamily = queueFamilies[familyIndex];

        bool hasRequiredFlags = (requiredFlags & queueFamily.queueFlags) == requiredFlags;
        bool supportSurface = device.getSurfaceSupportKHR(familyIndex, vk::SurfaceKHR(**mSurface)) == VK_TRUE;
        bool supportGranularity = queueFamily.minImageTransferGranularity.width == 1 &&
                                  queueFamily.minImageTransferGranularity.height == 1 &&
                                  queueFamily.minImageTransferGranularity.depth == 1;

        if (hasRequiredFlags && supportSurface && supportGranularity) {
            return familyIndex;
        }
    }

    return std::nullopt;
}