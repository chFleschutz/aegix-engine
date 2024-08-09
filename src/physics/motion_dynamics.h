#pragma once

#include "scripting/script_base.h"
#include "utils/math_utils.h"

namespace Aegix::Physics
{
	/// @brief Represents a directional and angular force.
	struct Force
	{
		Vector3 linear = Vector3{ 0.0f };
		Vector3 angular = Vector3{ 0.0f };

		Force operator+(const Force& other) const { return { linear + other.linear, angular + other.angular }; }
		Force operator-(const Force& other) const { return { linear - other.linear, angular - other.angular }; }
		Force operator*(const Force& other) const { return { linear * other.linear, angular * other.angular }; }
		Force operator/(const Force& other) const { return { linear / other.linear, angular / other.angular }; }
		Force operator*(float scalar) const { return { linear * scalar, angular * scalar }; }
		Force operator/(float scalar) const { return { linear / scalar, angular / scalar }; }

		Force& operator+=(const Force& other);
		Force& operator-=(const Force& other);
		Force& operator*=(const Force& other);
		Force& operator/=(const Force& other);
		Force& operator*=(float scalar);
		Force& operator/=(float scalar);
	};


	/// @brief Adds motion dynamics to an object to update its position and rotation each frame.
	class MotionDynamics : public Aegix::Scripting::ScriptBase
	{
	public:
		struct Properties
		{
			float mass = 1.0f;

			float linearFriction = 1.0f;
			float angularFriction = 1.0f;

			float maxLinearSpeed = 5.0f;
			float maxAngularSpeed = 5.0f;
		};

		/// @brief Adds a force to the object.
		/// @param force Directional and angular force to add.
		void addForce(const Force& force);

		/// @brief Adds a directional force to the object.
		/// @param force Directional force to add.
		void addLinearForce(const Vector3& force);

		/// @brief Adds an angular force to the object.
		/// @param angularForce Angular force to add.
		void addAngularForce(const Vector3& angularForce);

		/// @brief Halts all motion (sets velocities to zero).
		void haltMotion();

		/// @brief Returns the current directional velocity.
		Vector3 linearVelocity() const { return m_linearVelocity; }

		/// @brief Returns the current angular velocity.
		Vector3 angularVelocity() const { return m_angularVelocity; }

		/// @brief Returns the current directional speed.
		float linearSpeed() const { return glm::length(m_linearVelocity); }

		/// @brief Returns the current angular speed.
		float angularSpeed() const { return glm::length(m_angularVelocity); }

		/// @brief Returns a normalized vector in the current movement direction.
		Vector3 moveDirection() const;

		/// @brief Returns a normalized vector in the current angular direction.
		Vector3 angularDirection() const;

		Properties& properties() { return m_properties; }

		/// @brief Updates the position and rotation of the object based on the current velocity.
		/// @note This function is called automatically each frame.
		void update(float deltaSeconds) override;

	private:
		/// @brief Computes the velocities from the accelerations.
		/// @note Resets the accelerations to zero for the next frame.
		void applyForces(float deltaSeconds);

		void addFriction(float deltaSeconds);


		Properties m_properties;

		Vector3 m_linearVelocity{ 0.0f };
		Vector3 m_angularVelocity{ 0.0f };

		Vector3 m_accumulatedLinearForce{ 0.0f };
		Vector3 m_accumulatedAngularForce{ 0.0f };
	};
}
