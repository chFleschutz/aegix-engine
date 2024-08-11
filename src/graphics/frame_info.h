#pragma once

#include "camera.h"
#include "scene/scene.h"

#include <vulkan/vulkan.h>

namespace Aegix::Graphics
{
	struct GlobalLimits
	{
		static constexpr int MAX_LIGHTS = 10;
	};

	struct PointLight
	{
		Vector4 position{}; // ignore w
		Vector4 color{};	// w is intensity
	};

	struct GlobalUbo
	{
		Matrix4 projection{1.0f};
		Matrix4 view{1.0f};
		Matrix4 inverseView{1.0f};
		Vector4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f}; // w is the intesity
		PointLight pointLights[GlobalLimits::MAX_LIGHTS];
		int numLights;
	};

	struct FrameInfo
	{
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		Camera* camera;
		VkDescriptorSet globalDescriptorSet;
		Aegix::Scene::Scene* scene;
	};
}
