#pragma once

#include "graphics/vulkan/volk_include.h"
#include "core/logging.h"

#include <string_view>

namespace Aegis::Graphics
{
	class DebugUtilsMessenger
	{
	public:
		DebugUtilsMessenger() = default;
		~DebugUtilsMessenger() = default;

		void create();
		void destroy();

		static auto debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData) -> VkBool32;

		static auto populateCreateInfo() -> VkDebugUtilsMessengerCreateInfoEXT;
		static auto convertSeverity(VkDebugUtilsMessageSeverityFlagBitsEXT severity) -> ALOG::Severity;
		static auto convertMessageType(VkDebugUtilsMessageTypeFlagsEXT messageType) -> std::string_view;

	private:
		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	};
}