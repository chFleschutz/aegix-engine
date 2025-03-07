#pragma once

#include <random>

namespace Aegix
{
	class Random
	{
	public:
		/// @brief Seeds the random number generator
		static void seed(unsigned int seed) { s_generator.seed(seed); }

		/// @brief Returns the random number generator
		[[nodiscard]] auto static generator() -> std::mt19937& { return s_generator; }


		/// @brief Returns a uniform random float in the range [min, max]
		[[nodiscard]] static auto uniformFloat(float min = 0.0f, float max = 1.0f) -> float;

		/// @brief Returns a normal random float with the given mean and standard deviation
		[[nodiscard]] static auto normalFloat(float mean, float stddev) -> float;

		/// @brief Returns a normal random float in the range [min, max]
		[[nodiscard]] static auto normalFloatRange(float min = 0.0f, float max = 1.0f) -> float;


		/// @brief Returns a uniform random int in the range [min, max]
		[[nodiscard]] static auto uniformInt(int min, int max) -> int;

		/// @brief Returns a normal random int with the given mean and standard deviation
		[[nodiscard]] static auto normalInt(float mean, float stddev) -> int;

		/// @brief Returns a normal random int in the range [min, max]	
		[[nodiscard]] static auto normalIntRange(int min, int max) -> int;

	private:
		inline static std::mt19937 s_generator{ std::random_device{}() };
	};
}
