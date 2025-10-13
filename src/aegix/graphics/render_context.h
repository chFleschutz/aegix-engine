#pragma once

#include "scene/scene.h"
#include "ui/ui.h"

#include <vulkan/vulkan.h>

namespace Aegix::Graphics
{
	struct RenderContext
	{
		Scene::Scene& scene;
		UI::UI& ui;
		int frameIndex{ 0 };
		VkCommandBuffer cmd{ VK_NULL_HANDLE };
		VkDescriptorSet globalSet{ VK_NULL_HANDLE };
	};
}