#pragma once

#include "graphics/lights.h"
#include "scene/components.h"
#include "scene/scene.h"

namespace Aegix::Graphics
{
	struct GlobalLimits
	{
		static constexpr int MAX_LIGHTS = 10;
	};

	struct GlobalUbo
	{
		glm::mat4 projection{ 1.0f };
		glm::mat4 view{ 1.0f };
		glm::mat4 inverseView{ 1.0f };
	};

	struct FrameInfo
	{
		int frameIndex;
		VkCommandBuffer commandBuffer;
		Scene::Scene& scene;
		float aspectRatio;

		// TODO: Replace with a more general solution
		VkExtent2D swapChainExtend;
		VkImageView swapChainColor;
		VkImageView swapChainDepth;
	};
}
