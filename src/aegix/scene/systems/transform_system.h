#pragma once

#include "scene/system.h"

namespace Aegix::Scene
{
	class TransformSystem : public System
	{
	public:
		TransformSystem() = default;
		~TransformSystem() = default;

		void onBegin(Scene& scene) override;
		void onUpdate(float deltaSeconds, Scene& scene) override;
	};
}