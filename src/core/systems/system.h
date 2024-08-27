#pragma once

#include "scene/scene.h"

namespace Aegix
{
	class System
	{
	public:
		System() = default;
		virtual ~System() = default;

		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onUpdate(float deltaSeconds, Scene::Scene& scene) {}
	};
}