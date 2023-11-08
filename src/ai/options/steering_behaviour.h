#pragma once

#include "ai/options/option.h"
#include "utils/math_utils.h"

namespace vai
{
	class SteeringBehaviour : public Option
	{
	public:

	protected:
		virtual void updateOption(float deltaSeconds) override;

		virtual Vector3 computeAcceleration() = 0;
		virtual Vector3 computeAngularAcceleration() = 0;

	private:

	};

} // namespace vai