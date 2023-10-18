#pragma once

#include <random>

class Random
{
public:
	/// @brief Returns a uniform random float in the range [min, max]
	static float uniformFloat(float min = 0.0f, float max = 1.0f)
	{
		std::uniform_real_distribution distribution(min, max);
		return distribution(m_generator);
	}

	/// @brief Returns a normal random float with the given mean and standard deviation
	static float normalFloat(float mean, float stddev)
	{
		std::normal_distribution<float> distribution(mean, stddev);
		return distribution(m_generator);
	}

	/// @brief Returns a normal random float in the range [min, max]
	static float normalFloatRange(float min = 0.0f, float max = 1.0f)
	{
		return std::clamp(normalFloat((min + max) / 2.0f, (max - min) / 6.0f), min, max);
	}


	/// @brief Returns a uniform random int in the range [min, max]
	static int uniformInt(int min, int max)
	{
		std::uniform_int_distribution distribution(min, max);
		return distribution(m_generator);
	}

	/// @brief Returns a normal random int with the given mean and standard deviation
	static int normalInt(float mean, float stddev)
	{
		std::normal_distribution<float> distribution(mean, stddev);
		return std::lround(distribution(m_generator));
	}

	/// @brief Returns a normal random int in the range [min, max]	
	static int normalIntRange(int min, int max)
	{
		return std::clamp(normalInt((min + max) / 2.0f, (max - min) / 6.0f), min, max);
	}

private:
	static std::mt19937 m_generator;
};

// Initialize the random number generator
std::mt19937 Random::m_generator = std::mt19937(std::random_device()());
