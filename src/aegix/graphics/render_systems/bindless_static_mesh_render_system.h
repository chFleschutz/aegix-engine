#pragma once

#include "graphics/render_systems/render_system.h"
#include "graphics/resources/material_template.h"

namespace Aegix::Graphics
{
	class BindlessStaticMeshRenderSystem : public RenderSystem
	{
	public:
		static constexpr size_t MAX_OBJECT_COUNT = 100'000;

		struct PushConstantData
		{
			DescriptorHandle globalBuffer;
			DescriptorHandle objectBuffer;
			uint32_t objectIndex{ 0 };
			uint32_t frameIndex{ 0 };
		};

		struct ObjectData
		{
			glm::mat4 modelMatrix{ 1.0f };
			glm::mat4 normalMatrix{ 1.0f };
			DescriptorHandle meshHandle;
			DescriptorHandle materialHandle;
		};

		BindlessStaticMeshRenderSystem(MaterialType type = MaterialType::Opaque);

		virtual void render(const RenderContext& ctx) override;

	private:
		MaterialType m_type;
		Buffer m_objectBuffer;
	};
}