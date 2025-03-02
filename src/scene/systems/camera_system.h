#pragma once

#include "scene/components.h"
#include "scene/scene.h"
#include "scene/system.h"

namespace Aegix::Scene
{
	class CameraSystem : public System
	{
	public:
		virtual void onUpdate(float deltaSeconds, Scene& scene) override;

	private:
		void calcViewMatrix(Camera& camera, Transform& transform);
		void calcPerspectiveProjection(Camera& camera);
	};
}