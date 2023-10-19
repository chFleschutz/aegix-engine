#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cmath>

class MathLib
{
public:
	/// @brief Returns the percentage of a value between a min and max value
	static float percentage(float value, float min, float max)
	{
		return (value - min) / (max - min);
	}

	/// @brief Sigmoid function
	/// @param x Input value x between [0, 1]
	/// @return Returns x transformed by the sigmoid function between [0, 1]
	static float sigmoid(float x)
	{
		return 1.0f / (1.0f + std::exp(-12.0f * x + 6.0f));
	}

	/// @brief This is a fast approximation of the sigmoid function 
	/// @param x Input value x beetwen [0, 1]
	/// @return Returns x transformed by the sigmoid function between [0, 1] 
	static float fastSigmoid(float x)
	{
		x = 12.0f * x - 6.0f;
		return x / (1.0f + std::abs(x)) / 2.0f + 0.5f;
	}

	/// @brief Tangens hyperbolicus function transformed into the range [0, 1]
	/// @param x Input value x between [0, 1]
	/// @return Returns x transformed by the tanh function between [0, 1]
	static float tanh01(float x)
	{
		return std::tanh(6.0f * x - 3.0f) / 2.0f + 0.5f;
	}
};