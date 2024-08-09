#pragma once

#include "scripting/script_base.h"
#include "utils/math_utils.h"

#include <algorithm>

namespace Aegix::Scripting
{
	/// @brief Limits the locaton of the entity to the given limits.
	/// @note Only clamps the location, the velocity is not affected.
	class WorldBorder : public ScriptBase
	{
	public:
		/// @brief Constructs a WorldBorder.
		/// @param limits Positive values for the limits.
		WorldBorder(const Vector3& limits = Vector3{ FLT_MAX })
			: m_limits(limits) {}
		~WorldBorder() = default;

		virtual void update(float delta) override
		{
			auto& location = getComponent<Aegix::Component::Transform>().location;
			location.x = std::clamp(location.x, -m_limits.x, m_limits.x);
			location.y = std::clamp(location.y, -m_limits.y, m_limits.y);
			location.z = std::clamp(location.z, -m_limits.z, m_limits.z);
		}

	private:
		Vector3 m_limits;
	};
}
