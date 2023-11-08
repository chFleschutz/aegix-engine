#pragma once

#include "ai/options/steering_behaviour.h"

namespace vai
{
	class SteeringBehaviourSeek : public SteeringBehaviour
	{
	public:

	protected:
		void updateOption(float deltaSeconds) override;

	private:

	};
}