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
			auto view = scene.registry().view<Transform, Camera>();
			for (auto&& [entity, transform, camera] : view.each())
			{
				calcViewMatrix(camera, transform);
				calcPerspectiveProjection(camera);
			}
		}

	private:
		void calcViewMatrix(Camera& camera, Transform& transform);
		void calcPerspectiveProjection(Camera& camera);
	};
}