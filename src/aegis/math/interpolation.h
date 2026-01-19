#pragma once

#include "math/math.h"

namespace Aegis::Math
{
	template <typename T>
	T lerp(const T& a, const T& b, float t)
	{
		return a + t * (b - a);
	}

	/// @brief Returns the sigmoid function of x in the range [0, 1]
	auto sigmoid01(float x) -> float;
	
	/// @brief Returns the (fast approximation) sigmoid function of x in the range [0, 1]
	auto fastSigmoid01(float x) -> float;
	
	/// @brief Returns the tangens hyperbolicus of x in the range [0, 1]
	auto tanh01(float x) -> float;
}