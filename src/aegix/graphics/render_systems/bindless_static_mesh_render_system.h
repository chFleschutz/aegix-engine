#pragma once

#include "graphics/bindless/bindless_buffer.h"
#include "graphics/render_systems/render_system.h"
#include "graphics/material/material_template.h"

namespace Aegix::Graphics
{
	class BindlessStaticMeshRenderSystem : public RenderSystem
	{
	public:
		struct alignas(16) PushConstantData
		{
			glm::mat3x4 modelMatrix;
			glm::vec3 normalRow0; DescriptorHandle globalBuffer;
			glm::vec3 normalRow1; DescriptorHandle meshBuffer;
			glm::vec3 normalRow2; DescriptorHandle materialBuffer;
		};

		BindlessStaticMeshRenderSystem(MaterialType type = MaterialType::Opaque);

		virtual void render(const RenderContext& ctx) override;

	private:
		MaterialType m_type;
	};
}