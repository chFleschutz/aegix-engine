#pragma once

#include "graphics/vulkan/volk_include.h"
#include "scene/scene.h"
#include "ui/ui.h"

namespace Aegix::Graphics
{
	struct FrameInfo
	{
		Scene::Scene& scene;
		UI::UI& ui;
		VkCommandBuffer cmd{ VK_NULL_HANDLE };
		int frameIndex{ 0 };
		VkExtent2D swapChainExtent;
		float aspectRatio;
	};
}
