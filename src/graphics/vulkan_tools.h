#pragma once

#include "vulkan/vulkan.h"

#include <cassert>
#include <iostream>
#include <string_view>

#define CHECK_VK_RESULT(f)																				\
{																										\
	VkResult result = (f);																				\
	if (result != VK_SUCCESS)																			\
	{																									\
		std::cout << "Vulkan Error: " << Aegix::Tools::resultString(result) << " at " << __FILE__ << ", line " << __LINE__ << "\n"; \
		assert(result == VK_SUCCESS);																	\
	}																									\
}

namespace Aegix::Tools
{
	/// @brief Returns the result as a string
	std::string_view resultString(VkResult result);
}
