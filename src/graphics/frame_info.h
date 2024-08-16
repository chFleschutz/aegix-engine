#pragma once

#include "graphics/camera.h"
#include "graphics/lights.h"
#include "scene/scene.h"

namespace Aegix::Graphics
{
	struct GlobalLimits
	{
		static constexpr int MAX_LIGHTS = 10;
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
		VkCommandBuffer commandBuffer;
		Camera* camera;
		VkDescriptorSet globalDescriptorSet;
		Scene::Scene* scene;
	};
}
