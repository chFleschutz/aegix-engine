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
		VkCommandBuffer cmd{ VK_NULL_HANDLE };
		int frameIndex{ 0 };
	};
}