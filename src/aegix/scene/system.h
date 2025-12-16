#pragma once

#include <concepts>

namespace Aegix::Scene
{
	class Scene;

	class System
	{
	public:
		System() = default;
		virtual ~System() = default;

		virtual void onAttach() {}
		virtual void onDetach() {}
		virtual void onBegin(Scene& scene) {}
		virtual void onUpdate(float deltaSeconds, Scene& scene) {}
	};

	template <typename T>
	concept SystemDerived = std::derived_from<T, System>;
}