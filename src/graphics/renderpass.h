#pragma once

#include "graphics/device.h"
#include "scene/scene.h"

namespace Aegix::Graphics
{
	class Renderpass
	{
	public:
		Renderpass() = default;
		~Renderpass() = default;

		void render(VkCommandBuffer commandBuffer, Scene::Scene& scene);

	private:

	};
}