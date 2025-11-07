#pragma once

#include "graphics/bindless/bindless_buffer.h"
#include "graphics/render_systems/render_system.h"
#include "graphics/resources/material_template.h"

namespace Aegix::Graphics
{
	class BindlessStaticMeshRenderSystem : public RenderSystem
	{
	public:
		struct PushConstantData
		{
			glm::mat4 modelMatrix{ 1.0f };
			glm::mat4 normalMatrix{ 1.0f };
			DescriptorHandle globalBuffer;
			DescriptorHandle meshBuffer;
			DescriptorHandle materialBuffer;
		};

		BindlessStaticMeshRenderSystem(MaterialType type = MaterialType::Opaque);

		virtual void render(const RenderContext& ctx) override;

	private:
		MaterialType m_type;
	};
}