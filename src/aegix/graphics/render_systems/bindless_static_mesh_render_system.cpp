#include "pch.h"
#include "bindless_static_mesh_render_system.h"

#include "graphics/resources/material_instance.h"
#include "scene/components.h"

namespace Aegix::Graphics
{
	BindlessStaticMeshRenderSystem::BindlessStaticMeshRenderSystem(MaterialType type) : 
		m_type(type)
	{
	}

	void BindlessStaticMeshRenderSystem::render(const RenderContext& ctx)
	{
		// TODO: Sort for transparent materials back to front
		// TODO: Maybe also for opaque materials but front to back (avoid overdraw)

		MaterialTemplate* lastMatTemplate = nullptr;
		MaterialInstance* lastMatInstance = nullptr;
		uint32_t objectIndex = 0;
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
				currentMatTemplate->bindBindlessSet(ctx.cmd);
				lastMatTemplate = currentMatTemplate;
			}

			// Bind Descriptor Set
			if (lastMatInstance != material.instance.get())
			{
				material.instance->updateParameters(ctx.frameIndex);
				lastMatInstance = material.instance.get();
			}

			// Push Constants
			PushConstantData push{
				.modelMatrix = transform.matrix(),
				.normalMatrix = glm::transpose(glm::inverse(transform.matrix())),
				.globalBuffer = ctx.globalHandle,
				.meshBuffer = mesh.staticMesh->meshDataBuffer().handle(),
				.materialBuffer = material.instance->buffer().handle(ctx.frameIndex)
			};
			AGX_ASSERT_X(push.globalBuffer.isValid(), "Global buffer handle is invalid");
			AGX_ASSERT_X(push.meshBuffer.isValid(), "Mesh buffer handle is invalid");
			AGX_ASSERT_X(push.materialBuffer.isValid(), "Material buffer handle is invalid");

			currentMatTemplate->pushConstants(ctx.cmd, &push, sizeof(push));
			currentMatTemplate->draw(ctx.cmd, *mesh.staticMesh);
		}
	}
}
