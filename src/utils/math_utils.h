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

	/// @brief Simple linear interpolation between two values
	static float interpolate(float percentage, float min, float max)
	{
		return min + (max - min) * percentage;
	}

	/// @brief This is a fast approximation of the sigmoid function
	/// @param x Input value x
	/// @return Returns the sigmoid of x (closely approximated)
	static float fastSigmoid(float x)
	{
		// Taken from: https://stackoverflow.com/questions/10732027/fast-sigmoid-algorithm
		return x / (1.0f + std::abs(x));
	}

};