#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cmath>

// Aliases for glm types
using Vector2 = glm::vec2;
using Vector3 = glm::vec3;
using Vector4 = glm::vec4;
using Matrix2 = glm::mat2;
using Matrix3 = glm::mat3;
using Matrix4 = glm::mat4;

/// @brief MathLib contains some useful math functions
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
};