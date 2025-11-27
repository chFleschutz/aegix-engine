#pragma once

#include "graphics/render_systems/render_system.h"
#include "graphics/material/material_template.h"

namespace Aegix::Graphics
{
	class StaticMeshRenderSystem : public RenderSystem
	{
	public:
		struct PushConstantData
		{
			glm::mat4 modelMatrix{ 1.0f };
			glm::mat4 normalMatrix{ 1.0f };
		};

		StaticMeshRenderSystem(MaterialType type = MaterialType::Opaque);

		virtual void render(const RenderContext& ctx) override;

	private:
		MaterialType m_type;
	};
}