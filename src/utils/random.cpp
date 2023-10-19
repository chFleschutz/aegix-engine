#include "random.h"

// Initialize the random number generator
std::mt19937 Random::m_generator = std::mt19937(std::random_device()());

float Random::uniformFloat(float min, float max)
{
	std::uniform_real_distribution distribution(min, max);
	return distribution(m_generator);
}

float Random::normalFloat(float mean, float stddev)
{
	std::normal_distribution<float> distribution(mean, stddev);
	return distribution(m_generator);
}

float Random::normalFloatRange(float min, float max)
{
	return std::clamp(normalFloat((min + max) / 2.0f, (max - min) / 6.0f), min, max);
}

int Random::uniformInt(int min, int max)
{
	std::uniform_int_distribution distribution(min, max);
	return distribution(m_generator);
}

int Random::normalInt(float mean, float stddev)
{
	std::normal_distribution<float> distribution(mean, stddev);
	return std::lround(distribution(m_generator));
}

int Random::normalIntRange(int min, int max)
{
	return std::clamp(normalInt((min + max) / 2.0f, (max - min) / 6.0f), min, max);
}
