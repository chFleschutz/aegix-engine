#pragma once

#include "graphics/render_systems/render_system_registry.h"

#include <vulkan/vulkan.h>

namespace Aegix::Graphics
{
	struct FrameInfo
	{
		Scene::Scene& scene;
		UI::UI& ui;
		RenderSystemRegistry& renderSystems;
		VkCommandBuffer cmd{ VK_NULL_HANDLE };
		int frameIndex{ 0 };
		VkExtent2D swapChainExtent;
		float aspectRatio;
	};
}
