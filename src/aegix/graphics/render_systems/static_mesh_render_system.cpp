#include "static_mesh_render_system.h"

#include "graphics/resources/material_instance.h"
#include "scene/components.h"

namespace Aegix::Graphics
{
	StaticMeshRenderSystem::StaticMeshRenderSystem(MaterialType type)
		: m_type(type)
	{
	}

	void StaticMeshRenderSystem::render(const RenderContext& ctx)
	{
		// TODO: Sort for transparent materials back to front
		// TODO: Maybe also for opaque materials but front to back (avoid overdraw)

		MaterialTemplate* lastMatTemplate = nullptr;
		MaterialInstance* lastMatInstance = nullptr;

		auto view = ctx.scene.registry().view<GlobalTransform, Mesh, Material>();
		view.use<Material>();
		for (const auto& [entity, transform, mesh, material] : view.each())
		{
			if (!mesh.staticMesh || !material.instance)
				continue;

			auto currentMatTemplate = material.instance->materialTemplate().get();
			if (!currentMatTemplate || currentMatTemplate->type() != m_type)
				continue;

			// Bind Pipeline
			if (lastMatTemplate != currentMatTemplate)
			{
				currentMatTemplate->bind(ctx.cmd);
				currentMatTemplate->bindGlobalSet(ctx.cmd, ctx.globalSet);
				lastMatTemplate = currentMatTemplate;
			}

			// Bind Descriptor Set
			if (lastMatInstance != material.instance.get())
			{
				material.instance->bind(ctx.cmd, ctx.frameIndex);
				material.instance->updateParameters(ctx.frameIndex);
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
}
