#pragma once

#include "scene/components.h"
#include "scene/scene.h"

namespace Aegix::Graphics
{
	struct FrameInfo
	{
		int frameIndex;
		VkCommandBuffer commandBuffer;
		Scene::Scene& scene;
		float aspectRatio;
		VkExtent2D swapChainExtent;
	};
}
