#pragma once

#include "graphics/vulkan/volk_include.h"
#include "scene/scene.h"
#include "ui/ui.h"

namespace Aegix::Graphics
{
	struct RenderContext
	{
		Scene::Scene& scene;
		UI::UI& ui;
		uint32_t frameIndex{ 0 };
		VkCommandBuffer cmd{ VK_NULL_HANDLE };
		VkDescriptorSet globalSet{ VK_NULL_HANDLE };
	};
}