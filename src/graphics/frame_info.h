#pragma once

#include "graphics/lights.h"
#include "scene/components.h"
#include "scene/scene.h"

namespace Aegix::Graphics
{
	class RenderSystemCollection;

	struct GlobalLimits
	{
		static constexpr int MAX_LIGHTS = 10;
	};

	struct GlobalUbo
	{
		glm::mat4 projection{1.0f};
		glm::mat4 view{1.0f};
		glm::mat4 inverseView{1.0f};
		glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f}; // w is the intesity
		PointLight pointLights[GlobalLimits::MAX_LIGHTS];
		int numLights;
	};

	struct FrameInfo
	{
		int frameIndex;
		VkCommandBuffer commandBuffer;
		Scene::Scene& scene;
		float aspectRatio;
		VkDescriptorSet globalDescriptorSet;

		// TODO: Replace with a more general solution
		VkRenderPass swapChainRenderPass;
		VkExtent2D swapChainExtent;
		VkFramebuffer swapChainFramebuffer;
	};
}
