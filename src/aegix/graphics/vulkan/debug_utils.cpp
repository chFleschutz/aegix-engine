#include "debug_utils.h"

#include "graphics/device.h"
#include "graphics/vulkan/vulkan_context.h"
#include "graphics/vulkan/vulkan_tools.h"

namespace Aegix::Graphics
{
	void DebugUtilsMessenger::create()
	{
		if constexpr (ENABLE_VALIDATION)
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo = populateCreateInfo();
			VK_CHECK(vkCreateDebugUtilsMessengerEXT(VulkanContext::device().instance(), &createInfo, nullptr, &m_debugMessenger));
		}
	}

	void DebugUtilsMessenger::destroy()
	{
		if (ENABLE_VALIDATION)
		{
			vkDestroyDebugUtilsMessengerEXT(VulkanContext::device().instance(), m_debugMessenger, nullptr);
		}
	}

	auto DebugUtilsMessenger::populateCreateInfo() -> VkDebugUtilsMessengerCreateInfoEXT
	{
		VkDebugUtilsMessengerCreateInfoEXT info{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = debugCallback,
		};

		return info;
	}

	auto DebugUtilsMessenger::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
		void* pUserData) -> VkBool32
	{
		ALOG::log(convertSeverity(messageSeverity), "Vulkan {} Error: \n{}\n", 
			convertMessageType(messageType), pCallbackData->pMessage);
		return VK_FALSE;
	}

	auto DebugUtilsMessenger::convertSeverity(VkDebugUtilsMessageSeverityFlagBitsEXT severity) -> ALOG::Severity
	{
		switch (severity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			return ALOG::Severity::Trace;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			return ALOG::Severity::Info;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			return ALOG::Severity::Warn;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			return ALOG::Severity::Fatal;
		default:
			return ALOG::Severity::Info;
		}
	}

	auto DebugUtilsMessenger::convertMessageType(VkDebugUtilsMessageTypeFlagsEXT messageType) -> std::string_view
	{
		switch (messageType)
		{
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			return "General";
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			return "Validation";
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			return "Performance";
		default:
			return "Unknown";
		}
	}
}