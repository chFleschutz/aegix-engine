#include "steering_behaviour.h"

namespace VEAI
{
	void SteeringBehaviour::updateOption(float deltaSeconds)
	{
		auto acceleration = computeAcceleration();
		auto angularAcceleration = computeAngularAcceleration();

		// TODO: Update accelerations
		// auto& motionDynamics = getComponent<VEPhysics::MotionDynamics>();
		// motionDynamics.addAcceleration(acceleration);
		// motionDynamics.addAngularAcceleration(angularAcceleration);
	}

} // namespace VEAI