#pragma once

#include "scene/system.h"

namespace Aegix::Scene
{
	class TransformSystem : public System
	{
	public:
		TransformSystem() = default;
		~TransformSystem() = default;

		void onUpdate(float deltaSeconds, Scene& scene) override;
	};
}