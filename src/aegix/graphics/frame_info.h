#pragma once

#include "graphics/render_systems/render_system_registry.h"

#include <vulkan/vulkan.h>

namespace Aegix::Graphics
{
	struct FrameInfo
	{
		RenderSystemRegistry& renderSystems;
		VkExtent2D swapChainExtent;
		float aspectRatio;
	};
}
