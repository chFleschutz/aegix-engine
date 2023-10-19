#pragma once

#include <random>

class Random
{
public:
	/// @brief Returns a uniform random float in the range [min, max]
	static float uniformFloat(float min = 0.0f, float max = 1.0f);

	/// @brief Returns a normal random float with the given mean and standard deviation
	static float normalFloat(float mean, float stddev);

	/// @brief Returns a normal random float in the range [min, max]
	static float normalFloatRange(float min = 0.0f, float max = 1.0f);


	/// @brief Returns a uniform random int in the range [min, max]
	static int uniformInt(int min, int max);

	/// @brief Returns a normal random int with the given mean and standard deviation
	static int normalInt(float mean, float stddev);

	/// @brief Returns a normal random int in the range [min, max]	
	static int normalIntRange(int min, int max);

	/// @brief Seeds the random number generator
	static void seed(unsigned int seed);

private:
	static std::mt19937 m_generator;
};
