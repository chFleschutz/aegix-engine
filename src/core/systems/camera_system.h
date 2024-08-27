#pragma once

#include "core/systems/system.h"
#include "scene/components.h"

namespace Aegix
{
	class CameraSystem : public System
	{
	public:
		virtual void onUpdate(float deltaSeconds, Scene::Scene& scene) override
		{
			auto view = scene.viewEntities<Component::Transform, Component::Camera>();
			for (auto&& [entity, transform, camera] : view.each())
			{
				calcViewMatrix(camera, transform);
				calcPerspectiveProjection(camera);
			}
		}

	private:
		void calcViewMatrix(Component::Camera& camera, Component::Transform& transform);
		void calcPerspectiveProjection(Component::Camera& camera);
	};
}