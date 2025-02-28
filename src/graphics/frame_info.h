#pragma once

#include "ui/gui.h"
#include "scene/components.h"
#include "scene/scene.h"

namespace Aegix::Graphics
{
	struct FrameInfo
	{
		Scene::Scene& scene;
		GUI& gui;
		
		int frameIndex;
		VkCommandBuffer commandBuffer;
		VkExtent2D swapChainExtent;
		float aspectRatio;
	};
}
