#pragma once

#include "ui/ui.h"
#include "scene/components.h"
#include "scene/scene.h"

namespace Aegix::Graphics
{
	struct FrameInfo
	{
		Scene::Scene& scene;
		UI::UI& ui;
		int frameIndex;
		VkCommandBuffer commandBuffer;
		VkExtent2D swapChainExtent;
		float aspectRatio;
	};
}
