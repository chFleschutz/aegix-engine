#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>


// Aliases for glm types
using Vector2 = glm::vec2;
using Vector3 = glm::vec3;
using Vector4 = glm::vec4;
using Matrix2 = glm::mat2;
using Matrix3 = glm::mat3;
using Matrix4 = glm::mat4;

// Overloads to print glm types
std::ostream& operator<<(std::ostream& os, const Vector2& vec);
std::ostream& operator<<(std::ostream& os, const Vector3& vec);
std::ostream& operator<<(std::ostream& os, const Vector4& vec);


namespace Aegix
{
	/// @brief Aegix::MathLib contains some useful math functions
	class MathLib
	{
	public:
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
		static Matrix4 tranformationMatrix(const Vector3& location, const Vector3& rotation, const Vector3& scale);

		/// @brief Compoeses a matrix to transform normals into world space
		/// @return Composed normal matrix
		static Matrix3 normalMatrix(const Vector3& rotation, const Vector3& scale);

		/// @brief Returns the global forward direction
		static Vector3 forward() { return { 0.0f, 0.0f, 1.0f }; }

		/// @brief Returns the global right direction
		static Vector3 right() { return { 1.0f, 0.0f, 0.0f }; }

		/// @brief Returns the global up direction
		static Vector3 up() { return { 0.0f, -1.0f, 0.0f }; }

		/// @brief Returns the forward direction of a rotation
		/// @param rotation Rotation in euler angles
		/// @return Forward direction of the rotation
		static Vector3 forward(const Vector3& rotation);

		/// @brief Returns the right direction of a rotation
		/// @param rotation Rotation in euler angles
		/// @return Right direction of the rotation
		static Vector3 right(const Vector3& rotation);

		/// @brief Returns the up direction of a rotation
		/// @param rotation Rotation in euler angles
		/// @return Up direction of the rotation
		static Vector3 up(const Vector3& rotation);

		/// @brief Normalizes a vector
		/// @param vec Non zero vector to normalize (length > 0)
		/// @return Returns the normalized vector
		static Vector3 normalize(const Vector3& vec);

		/// @brief Checks if the target is in the field of view of the view direction
		/// @param viewDirection Vector pointing in the direction of the view
		/// @param targetDirection Vector pointing in the direction of the target
		/// @param fov Field of view in radians
		/// @return Returns true if the target is in the field of view otherwise false
		static bool inFOV(const Vector3& viewDirection, const Vector3& targetDirection, float fov);
	};
}
