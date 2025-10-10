#pragma once

#include "graphics/render_systems/render_system.h"

namespace Aegix::Graphics
{
	struct PushConstantData
	{
		glm::mat4 modelMatrix{ 1.0f };
		glm::mat4 normalMatrix{ 1.0f };
	};

	enum class ObjectType
	{
		Opaque = 0,
		Transparent = 1
	};

	class StaticMeshRenderSystem : public RenderSystem
	{
	public:
		StaticMeshRenderSystem(ObjectType objectType = ObjectType::Opaque)
			: m_objectType{ objectType }
		{
		}

		virtual void render(const RenderContext& ctx) override
		{
			// TODO: use ObjectType to filter objects

			MaterialTemplate* lastMatTemplate = nullptr;
			MaterialInstance* lastMatInstance = nullptr;

			auto view = ctx.scene.registry().view<GlobalTransform, Mesh, Material>();
			view.use<Material>();
			for (const auto& [entity, transform, mesh, material] : view.each())
			{
				if (!mesh.staticMesh || !material.instance)
					continue;

				auto currentMatTemplate = material.instance->materialTemplate().get();
				if (!currentMatTemplate)
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
					material.instance->bind(ctx.cmd);
					material.instance->updateParameters();
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

	private:
		ObjectType m_objectType;
	};
}