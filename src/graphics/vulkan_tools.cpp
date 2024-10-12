#include "vulkan_tools.h"


namespace Aegix::Tools
{
	std::string_view resultString(VkResult result)
	{
		switch (result)
		{
#define CASE(name) case name: return #name
			CASE(VK_SUCCESS);
			CASE(VK_NOT_READY);
			CASE(VK_TIMEOUT);
			CASE(VK_EVENT_SET);
			CASE(VK_EVENT_RESET);
			CASE(VK_INCOMPLETE);
			CASE(VK_ERROR_OUT_OF_HOST_MEMORY);
			CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY);
			CASE(VK_ERROR_INITIALIZATION_FAILED);
			CASE(VK_ERROR_DEVICE_LOST);
			CASE(VK_ERROR_MEMORY_MAP_FAILED);
			CASE(VK_ERROR_LAYER_NOT_PRESENT);
			CASE(VK_ERROR_EXTENSION_NOT_PRESENT);
			CASE(VK_ERROR_FEATURE_NOT_PRESENT);
			CASE(VK_ERROR_INCOMPATIBLE_DRIVER);
			CASE(VK_ERROR_TOO_MANY_OBJECTS);
			CASE(VK_ERROR_FORMAT_NOT_SUPPORTED);
			CASE(VK_ERROR_FRAGMENTED_POOL);
			CASE(VK_ERROR_UNKNOWN);
			CASE(VK_ERROR_OUT_OF_POOL_MEMORY);
			CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE);
			CASE(VK_ERROR_FRAGMENTATION);
			CASE(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
			CASE(VK_PIPELINE_COMPILE_REQUIRED);
			CASE(VK_ERROR_SURFACE_LOST_KHR);
			CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
			CASE(VK_SUBOPTIMAL_KHR);
			CASE(VK_ERROR_OUT_OF_DATE_KHR);
			CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
			CASE(VK_ERROR_VALIDATION_FAILED_EXT);
			CASE(VK_ERROR_INVALID_SHADER_NV);
			CASE(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
			CASE(VK_ERROR_NOT_PERMITTED_KHR);
			CASE(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
			CASE(VK_THREAD_IDLE_KHR);
			CASE(VK_THREAD_DONE_KHR);
			CASE(VK_OPERATION_DEFERRED_KHR);
			CASE(VK_OPERATION_NOT_DEFERRED_KHR);
			CASE(VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR);
			CASE(VK_ERROR_COMPRESSION_EXHAUSTED_EXT);
			CASE(VK_INCOMPATIBLE_SHADER_BINARY_EXT);
#undef CASE
		default:
			return "RESULT_UNKNOWN";
		}
	}

	bool Aegix::Tools::isDepthFormat(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return true;
		default:
			return false;
		}
	}
}
