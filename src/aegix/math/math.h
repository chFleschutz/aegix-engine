#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <cmath>

namespace Aegix::Math
{
	namespace World
	{
		constexpr glm::vec3 RIGHT = { 1.0f, 0.0f, 0.0f };
		constexpr glm::vec3 FORWARD = { 0.0f, 1.0f, 0.0f };
		constexpr glm::vec3 UP = { 0.0f, 0.0f, 1.0f };
	}

	/// @brief Returns the forward direction of a rotation
	auto forward(const glm::vec3& rotation) -> glm::vec3;
	auto forward(const glm::quat& rotation) -> glm::vec3;

	/// @brief Returns the right direction of a rotation
	auto right(const glm::vec3& rotation) -> glm::vec3;
	auto right(const glm::quat& rotation) -> glm::vec3;

	/// @brief Returns the up direction of a rotation
	auto up(const glm::vec3& rotation) -> glm::vec3;
	auto up(const glm::quat& rotation) -> glm::vec3;

	/// @brief Composes a transformation matrix from location, rotation and scale
	auto tranformationMatrix(const glm::vec3& location, const glm::vec3& rotation, const glm::vec3& scale) -> glm::mat4;
	auto tranformationMatrix(const glm::vec3& location, const glm::quat& rotation, const glm::vec3& scale) -> glm::mat4;

	/// @brief Composes a matrix to transform normals into world space
	auto normalMatrix(const glm::vec3& rotation, const glm::vec3& scale) -> glm::mat3;
	auto normalMatrix(const glm::quat& rotation, const glm::vec3& scale) -> glm::mat3;

	/// @brief Decomposes a transformation matrix into location, rotation and scale
	void decomposeTRS(const glm::mat4& matrix, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
	void decomposeTRS(const glm::mat4& matrix, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale);

	/// @brief Returns the percentage of a value between a min and max value
	auto percentage(float value, float min, float max) -> float;

	/// @brief Checks if the target is in the field of view of the view direction
	auto inFOV(const glm::vec3& viewDirection, const glm::vec3& targetDirection, float fov) -> bool;
}
