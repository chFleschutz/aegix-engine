#pragma once

#include "scripting/script_base.h"
#include "utils/math_utils.h"

namespace VEPhysics
{
	/// @brief Adds motion dynamics to an object to update its position and rotation each frame.
	class MotionDynamics : public VEScripting::ScriptBase
	{
	public:
		/// @brief Adds a directional acceleration to the object.
		/// @param acceleration Directional acceleration to add.
		void addAcceleration(const Vector3& acceleration);

		/// @brief Adds an angular acceleration to the object.
		/// @param angularAcceleration Angular acceleration to add.
		void addAngularAcceleration(const Vector3& angularAcceleration);

		/// @brief Returns the current directional velocity.
		Vector3 velocity() const { return m_velocity; }

		/// @brief Returns the current angular velocity.
		Vector3 angularVelocity() const { return m_angularVelocity; }

		/// @brief Returns the current directional speed.
		float speed() const { return glm::length(m_velocity); }

		/// @brief Returns the current angular speed.
		float angularSpeed() const { return glm::length(m_angularVelocity); }

		/// @brief Returns a normalized vector in the current movement direction.
		Vector3 moveDirection() const { return glm::normalize(m_velocity); }

		/// @brief Updates the position and rotation of the object based on the current velocity.
		/// @note This function is called automatically each frame.
		void update(float deltaSeconds) override;

	private:
		/// @brief Computes the velocities from the accelerations.
		/// @note Resets the accelerations to zero for the next frame.
		void applyAcceleration(float deltaSeconds);

		Vector3 m_velocity{ 0.0f };
		Vector3 m_angularVelocity{ 0.0f };

		Vector3 m_acceleration{ 0.0f };
		Vector3 m_angularAcceleration{ 0.0f };
	};

} // namespace VEPhysics