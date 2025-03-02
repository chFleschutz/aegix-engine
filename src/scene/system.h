#pragma once

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
		virtual void onUpdate(float deltaSeconds, Scene& scene) {}
	};

	template <typename T>
	concept SystemDerived = std::derived_from<T, System>;
}