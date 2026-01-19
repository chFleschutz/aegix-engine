#include "pch.h"
#include "transform_system.h"

#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"

namespace Aegis::Scene
{
	void TransformSystem::onBegin(Scene& scene)
	{
		// Calculate initial global transforms for all entities
		std::vector<Entity> hierarchy;
		auto view = scene.registry().view<GlobalTransform>();
		for (auto&& [entity, globalTransform] : view.each())
		{
			// Find hierarchy of all parents
			hierarchy.clear();
			Entity current = Entity{ entity, &scene };
			while (current)
			{
				hierarchy.emplace_back(current);
				current = current.get<Parent>().entity;
			}

			globalTransform = GlobalTransform{};
			for (auto e : std::ranges::views::reverse(hierarchy))
			{
				auto& localTransform = e.get<Transform>();
				globalTransform.location = globalTransform.location + localTransform.location;
				globalTransform.rotation = globalTransform.rotation * localTransform.rotation;
				globalTransform.scale = globalTransform.scale * localTransform.scale;
			}
		}
	}

	void TransformSystem::onUpdate(float deltaSeconds, Scene& scene)
	{
		// Note: This lags 1 frame per parent behind the actual local transform (but this is fine :)
		auto view = scene.registry().view<Transform, GlobalTransform, Parent, DynamicTag>();
		for (auto&& [entity, transform, globalTransform, parent] : view.each())
		{
			if (!parent.entity || !parent.entity.has<GlobalTransform>())
			{
				globalTransform.location = transform.location;
				globalTransform.rotation = transform.rotation;
				globalTransform.scale = transform.scale;
				continue;
			}

			auto& parentGlobal = parent.entity.get<GlobalTransform>();
			globalTransform.location = parentGlobal.location + transform.location;
			globalTransform.rotation = parentGlobal.rotation * transform.rotation;
			globalTransform.scale = parentGlobal.scale * transform.scale;
		}
	}
}
