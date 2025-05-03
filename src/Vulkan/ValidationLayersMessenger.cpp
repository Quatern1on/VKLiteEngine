#include "pch.h"
#include "ValidationLayersMessenger.h"

std::string formatMessageType(VkDebugUtilsMessageTypeFlagsEXT messageType) {
    switch (messageType) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            return "General";

        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            return "Validation";

        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            return "Performance";

        case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
            return "Device address binding";
        default:
            return "Undefined";
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL ValidationLayersMessenger::debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

    std::string messageTypeString = formatMessageType(messageType);

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        VLOG(1) << "Layer message (" << messageTypeString << ")";
        VLOG(1) << pCallbackData->pMessage;
    } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        LOG(INFO) << "Layer message (" << messageTypeString << ")";
        LOG(INFO) << pCallbackData->pMessage;
    } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        LOG(WARNING) << "Layer message (" << messageTypeString << ")";
        LOG(WARNING) << "\033[33m" << pCallbackData->pMessage << "\033[0m";
    } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        LOG(ERROR) << "Layer message (" << messageTypeString << ")";
        LOG(ERROR) << "\033[31m" << pCallbackData->pMessage << "\033[0m";
    }

    return VK_FALSE;
}