#pragma once

#include "scripting/script_base.h"
#include "utils/math_utils.h"

namespace VEPhysics
{
	/// @brief Adds motion dynamics to an object to update its position and rotation each frame.
	class MotionDynamics : public VEScripting::ScriptBase
	{
	public:
		/// @brief Adds a directional force to the object.
		/// @param force Directional force to add.
		void addForce(const Vector3& force);

		/// @brief Adds an angular force to the object.
		/// @param angularForce Angular force to add.
		void addAngularForce(const Vector3& angularForce);

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

		/// @brief Sets the mass.
		void setMass(float mass) { m_mass = mass; }

		/// @brief Returns the mass.
		float mass() const { return m_mass; }

		/// @brief Updates the position and rotation of the object based on the current velocity.
		/// @note This function is called automatically each frame.
		void update(float deltaSeconds) override;

	private:
		/// @brief Computes the velocities from the accelerations.
		/// @note Resets the accelerations to zero for the next frame.
		void applyForces(float deltaSeconds);

		float m_mass{ 1.0f };

		Vector3 m_velocity{ 0.0f };
		Vector3 m_angularVelocity{ 0.0f };

		Vector3 m_accumulatedForce{ 0.0f };
		Vector3 m_accumulatedAngularForce{ 0.0f };
	};

} // namespace VEPhysics