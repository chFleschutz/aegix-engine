#include "pch.h"
#include "random.h"

namespace Aegis
{
	auto Random::uniformFloat(float min, float max) -> float
	{
		std::uniform_real_distribution distribution(min, max);
		return distribution(s_generator);
	}

	auto Random::normalFloat(float mean, float stddev) -> float
	{
		std::normal_distribution<float> distribution(mean, stddev);
		return distribution(s_generator);
	}

	auto Random::normalFloatRange(float min, float max) -> float
	{
		return std::clamp(normalFloat((min + max) / 2.0f, (max - min) / 6.0f), min, max);
	}

	auto Random::uniformInt(int min, int max) -> int
	{
		std::uniform_int_distribution distribution(min, max);
		return distribution(s_generator);
	}

	auto Random::normalInt(float mean, float stddev) -> int
	{
		std::normal_distribution<float> distribution(mean, stddev);
		return std::lround(distribution(s_generator));
	}

	auto Random::normalIntRange(int min, int max) -> int
	{
		return std::clamp(normalInt((min + max) / 2.0f, (max - min) / 6.0f), min, max);
	}
}
