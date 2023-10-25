#include "math_utils.h"


float MathLib::percentage(float value, float min, float max)
{
	return (value - min) / (max - min);
}

float MathLib::sigmoid01(float x)
{
	return 1.0f / (1.0f + std::exp(-12.0f * x + 6.0f));
}

float MathLib::fastSigmoid01(float x)
{
	x = 12.0f * x - 6.0f;
	return x / (1.0f + std::abs(x)) / 2.0f + 0.5f;
}

float MathLib::tanh01(float x)
{
	return std::tanh(6.0f * x - 3.0f) / 2.0f + 0.5f;
}
