#pragma once

#include <vulkan/vulkan.h>

namespace Aegix::Graphics
{
	struct FrameInfo
	{
		VkExtent2D swapChainExtent;
		float aspectRatio;
	};
}
