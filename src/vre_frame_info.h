#pragma once

#include "vre_camera.h"
#include "vre_game_object.h"

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
		VreGameObject::Map& gameObjects;
	};

} // namespace vre