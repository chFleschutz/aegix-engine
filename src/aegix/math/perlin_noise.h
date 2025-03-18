#pragma once

namespace Aegix
{
	/// @brief 1D Perlin noise generator.
	class PerlinNoise1D
	{
	public:
		/// @brief Creates a new Perlin noise random number generator.
		/// @note The noise is generated using the Random class, for the same result seed it BEFORE creating PerlinNoise1D object.
		explicit PerlinNoise1D(float intervalSize = 1.0f);

		/// @brief Returns the noise value at the given position.
		float noise(float x, int rank, float persistence = 0.5f);

	private:
		/// @brief Octave holds the signal values of a single octave with more values (higher LOD) per rank.
		struct Octave
		{
			Octave(int rank, float firstValue, float lastValue);

			/// @brief Returns the value of the octave at the given position.
			/// @note Relative position x MUST be between [0, 1).
			float value(float x) const;

			int rank;
			std::vector<float> signalValues;
		};

		/// @brief Represents an interval of the noise with multiple octaves.
		/// @note The length of the interval is the bandwidth of the noise but each octave uses relative values between [0, 1).
		struct Interval
		{
			Interval(float firstValue);

			void addOctave(float firstValue, float lastValue);

			std::vector<Octave> octaves;
		};

		void addInterval();
		void addOctave(int intervalIndex);

		float m_bandwidth;
		std::vector<Interval> m_intervals;
	};
}
