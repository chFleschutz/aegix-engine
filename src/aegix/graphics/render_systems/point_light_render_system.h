#pragma once

#include "graphics/render_systems/render_system.h"
#include "graphics/pipeline.h"

namespace Aegis::Graphics
{
	class PointLightRenderSystem : public RenderSystem
	{
	public:
		struct PointLightPushConstants // max 128 bytes
		{
			glm::vec4 position{};
			glm::vec4 color{};
			float radius;
		};

		PointLightRenderSystem();

		virtual void render(const RenderContext& ctx) override;

	private:
		std::unique_ptr<Pipeline> m_pipeline;
	};
}