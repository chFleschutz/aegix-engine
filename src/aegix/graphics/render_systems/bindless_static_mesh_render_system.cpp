#include "pch.h"
#include "bindless_static_mesh_render_system.h"

#include "graphics/resources/material_instance.h"
#include "scene/components.h"

namespace Aegix::Graphics
{
	BindlessStaticMeshRenderSystem::BindlessStaticMeshRenderSystem(MaterialType type) : 
		m_type(type),
		m_objectBuffer{ Buffer::createStorageBuffer(sizeof(ObjectData), MAX_OBJECT_COUNT) }
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

			ObjectData objData{
				.modelMatrix = transform.matrix(),
				.normalMatrix = Math::normalMatrix(transform.rotation, transform.scale),
				.meshHandle = mesh.staticMesh->meshDataBuffer().handle(),
				.materialHandle = material.instance->buffer().handle()
			};
			AGX_ASSERT_X(objData.meshHandle.isValid(), "Invalid mesh handle in BindlessStaticMeshRenderSystem!");
			AGX_ASSERT_X(objData.materialHandle.isValid(), "Invalid material handle in BindlessStaticMeshRenderSystem!");
			m_objectBuffer.singleWrite(&objData, sizeof(ObjectData), objectIndex * sizeof(ObjectData));

			PushConstantData push{
				.globalBuffer = ctx.globalHandle,
				.objectBuffer = m_objectBuffer.handle(),
				.objectIndex = objectIndex++,
				.frameIndex = ctx.frameIndex
			};
			AGX_ASSERT_X(push.globalBuffer.isValid(), "Invalid global buffer handle in BindlessStaticMeshRenderSystem!");
			AGX_ASSERT_X(push.objectBuffer.isValid(), "Invalid object buffer handle in BindlessStaticMeshRenderSystem!");
			currentMatTemplate->pushConstants(ctx.cmd, &push, sizeof(push));
			currentMatTemplate->draw(ctx.cmd, *mesh.staticMesh);
		}
	}
}
