#pragma once

#include "vre_camera.h"

#include <vulkan/vulkan.h>

namespace vre
{
	struct FrameInfo
	{
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VreCamera& camera;
		VkDescriptorSet globalDescriptorSet;
	};

} // namespace vre