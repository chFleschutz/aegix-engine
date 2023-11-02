#pragma once

#include "utils/math_utils.h"

#include <vector>

/// @brief 1D Perlin noise generator.
class PerlinNoise1D
{
public:
	/// @brief Creates a new Perlin noise random number generator.
	/// @param intervalSize Size of the interval in which the noise is generated.
	/// @note The Random class is used to generate the noise, if you want to have the same noise seed it BEFORE creating.
	explicit PerlinNoise1D(float intervalSize = 1.0f);

	/// @brief Returns the noise value at the given position.
	/// @param x Position.
	/// @param rank Rank of highest octave to use (more LOD but more expensive).
	/// @param persistence Persistence of the noise (keep below 0.5 or return values can be greater 1).
	float noise(float x, int rank, float persistence = 0.5f);

private:
	struct Octave
	{
		Octave(int rank, float firstValue, float lastValue);

		/// @brief Returns the value of the octave at the given position.
		/// @param x Relative position in the octave between [0, 1).
		float value(float x) const;

		int rank;
		std::vector<Vector2> signalValues;
	};

	struct Interval
	{
		Interval(float firstValue);
		
		float lastValue() const { return octaves.front().signalValues.back().y; }
		void addOctave(float firstValue, float lastValue);

		std::vector<Octave> octaves;
	};

	void addInterval(float firstValue);
	void addOctave(int intervalIndex);

	float m_bandwidth;
	std::vector<Interval> m_intervals;
};
