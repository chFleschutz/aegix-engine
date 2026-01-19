#pragma once

#include "scene/scene.h"

namespace Aegis::Scene
{
	/// @brief Base class for creating a scene
	class Description
	{
	public:
		virtual ~Description() = default;

		/// @brief All objects in a scene are created here
		virtual void initialize(Scene& scene) = 0;
	};

	template<typename T>
	concept DescriptionDerived = std::derived_from<T, Description>;
}
