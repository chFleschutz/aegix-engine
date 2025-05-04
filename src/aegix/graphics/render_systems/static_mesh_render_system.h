#pragma once

#include "graphics/render_systems/render_system.h"

namespace Aegix::Graphics
{
	struct PushConstantData
	{
		glm::mat4 modelMatrix{ 1.0f };
		glm::mat4 normalMatrix{ 1.0f };
	};

	class StaticMeshRenderSystem : public RenderSystemBase
	{
	public:
		virtual void render(const RenderContext& ctx) override
		{
			MaterialTemplate* lastMatTemplate = nullptr;
			MaterialInstance* lastMatInstance = nullptr;

			auto view = ctx.scene.registry().view<GlobalTransform, Mesh, Material>();
			view.use<Material>();
			for (const auto& [entity, transform, mesh, material] : view.each())
			{
				if (!mesh.staticMesh || !material.instance)
					continue;

				// Bind Pipeline
				auto currentMatTemplate = material.instance->materialTemplate().get();
				if (lastMatTemplate != currentMatTemplate)
				{
					currentMatTemplate->bind(ctx.cmd);
					lastMatTemplate = currentMatTemplate;
				}

				// Bind Descriptor Set
				if (lastMatInstance != material.instance.get())
				{
					material.instance->bind(ctx.cmd);
					lastMatInstance = material.instance.get();
				}

				// Push Constants
				PushConstantData push{
					.modelMatrix = transform.matrix(),
					.normalMatrix = Math::normalMatrix(transform.rotation, transform.scale)
				};
				currentMatTemplate->pushConstants(ctx.cmd, &push, sizeof(push));

				// Bind and Draw Mesh
				mesh.staticMesh->bind(ctx.cmd);
				mesh.staticMesh->draw(ctx.cmd);
			}
		}
	};
}