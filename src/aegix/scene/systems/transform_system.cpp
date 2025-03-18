#include "pch.h"

#include "transform_system.h"

#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"

namespace Aegix::Scene
{
	void TransformSystem::onUpdate(float deltaSeconds, Scene& scene)
	{
		// Note: This lags 1 frame per parent behind the actual local transform (but this is fine)
		auto view = scene.registry().view<Transform, GlobalTransform, Parent>();
		for (auto&& [entity, transform, globalTransform, parent] : view.each())
		{
			if (!parent.entity || !parent.entity.hasComponent<GlobalTransform>())
			{
				globalTransform.location = transform.location;
				globalTransform.rotation = transform.rotation;
				globalTransform.scale = transform.scale;
				continue;
			}

			auto& parentGlobal = parent.entity.component<GlobalTransform>();
			globalTransform.location = parentGlobal.location + transform.location;
			globalTransform.rotation = parentGlobal.rotation * transform.rotation;
			globalTransform.scale = parentGlobal.scale * transform.scale;
		}
	}
}
