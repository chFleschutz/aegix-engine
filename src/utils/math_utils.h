#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>


// Overloads to print glm types
std::ostream& operator<<(std::ostream& os, const glm::vec2& vec);
std::ostream& operator<<(std::ostream& os, const glm::vec3& vec);
std::ostream& operator<<(std::ostream& os, const glm::vec4& vec);


namespace Aegix
{
	/// @brief Aegix::MathLib contains some useful math functions
	class MathLib
	{
	public:
		constexpr static glm::vec3 RIGHT = { 1.0f, 0.0f, 0.0f };
		constexpr static glm::vec3 FORWARD = { 0.0f, 1.0f, 0.0f };
		constexpr static glm::vec3 UP = { 0.0f, 0.0f, 1.0f };

		/// @brief Returns the percentage of a value between a min and max value
		static float percentage(float value, float min, float max);

		/// @brief Sigmoid function
		/// @param x Input value x between [0, 1]
		/// @return Returns x transformed by the sigmoid function between [0, 1]
		static float sigmoid01(float x);

		/// @brief This is a fast approximation of the sigmoid function 
		/// @param x Input value x beetwen [0, 1]
		/// @return Returns x transformed by the sigmoid function between [0, 1] 
		static float fastSigmoid01(float x);

		/// @brief Tangens hyperbolicus function transformed into the range [0, 1]
		/// @param x Input value x between [0, 1]
		/// @return Returns x transformed by the tanh function between [0, 1]
		static float tanh01(float x);

		/// @brief Composes a transformation matrix from location, rotation and scale
		/// @return Composed transformation matrix
		static glm::mat4 tranformationMatrix(const glm::vec3& location, const glm::vec3& rotation, const glm::vec3& scale);
		static glm::mat4 tranformationMatrix(const glm::vec3& location, const glm::quat& rotation, const glm::vec3& scale);

		/// @brief Compoeses a matrix to transform normals into world space
		/// @return Composed normal matrix
		static glm::mat3 normalMatrix(const glm::vec3& rotation, const glm::vec3& scale);
		static glm::mat3 normalMatrix(const glm::quat& rotation, const glm::vec3& scale);

		/// @brief Decomposes a transformation matrix into location, rotation and scale
		/// @note The matrix has to consist of only translation, rotation and scale (M = T * R * S)
		static void decomposeTRS(const glm::mat4& matrix, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
		static void decomposeTRS(const glm::mat4& matrix, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale);

		/// @brief Returns the global forward direction
		static glm::vec3 forward() { return { 0.0f, 0.0f, 1.0f }; }

		/// @brief Returns the global right direction
		static glm::vec3 right() { return { 1.0f, 0.0f, 0.0f }; }

		/// @brief Returns the global up direction
		static glm::vec3 up() { return { 0.0f, -1.0f, 0.0f }; }

		/// @brief Returns the forward direction of a rotation
		/// @param rotation Rotation in euler angles
		/// @return Forward direction of the rotation
		static glm::vec3 forward(const glm::vec3& rotation);
		static glm::vec3 forward(const glm::quat& rotation);

		/// @brief Returns the right direction of a rotation
		/// @param rotation Rotation in euler angles
		/// @return Right direction of the rotation
		static glm::vec3 right(const glm::vec3& rotation);
		static glm::vec3 right(const glm::quat& rotation);

		/// @brief Returns the up direction of a rotation
		/// @param rotation Rotation in euler angles
		/// @return Up direction of the rotation
		static glm::vec3 up(const glm::vec3& rotation);
		static glm::vec3 up(const glm::quat& rotation);

		/// @brief Normalizes a vector
		/// @param vec Non zero vector to normalize (length > 0)
		/// @return Returns the normalized vector
		static glm::vec3 normalize(const glm::vec3& vec);

		/// @brief Checks if the target is in the field of view of the view direction
		/// @param viewDirection Vector pointing in the direction of the view
		/// @param targetDirection Vector pointing in the direction of the target
		/// @param fov Field of view in radians
		/// @return Returns true if the target is in the field of view otherwise false
		static bool inFOV(const glm::vec3& viewDirection, const glm::vec3& targetDirection, float fov);
	};
}
