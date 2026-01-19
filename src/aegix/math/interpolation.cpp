#include "pch.h"
#include "interpolation.h"

namespace Aegis::Math
{
	auto sigmoid01(float x) -> float
	{
		return 1.0f / (1.0f + std::exp(-12.0f * x + 6.0f));
	}

	auto fastSigmoid01(float x) -> float
	{
		x = 12.0f * x - 6.0f;
		return x / (1.0f + std::abs(x)) / 2.0f + 0.5f;
	}

	auto tanh01(float x) -> float
	{
		return std::tanh(6.0f * x - 3.0f) / 2.0f + 0.5f;
	}
}
