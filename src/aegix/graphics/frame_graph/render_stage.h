#pragma once

#include "graphics/descriptors.h"

namespace Aegix::Graphics
{
	class RenderSystem;

	struct RenderStage
	{
		enum class Type
		{
			Geometry,
			Transparency,
			Count
		};

		std::vector<std::unique_ptr<RenderSystem>> renderSystems;
		std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
		std::unique_ptr<DescriptorSet> descriptorSet;
		std::unique_ptr<UniformBuffer> ubo;
	};
}