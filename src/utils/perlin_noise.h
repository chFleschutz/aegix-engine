#pragma once

#include "utils/math_utils.h"

#include <vector>

/// @brief 1D Perlin noise generator.
class PerlinNoise1D
{
public:
	PerlinNoise1D();

	/// @brief Returns the noise value at the given position.
	/// @param x Position.
	/// @param rank Rank of highest octave to use (more LOD but more expensive).
	/// @param persistence Persistence of the noise (keep below 0.5 or return values can be greater 1).
	float noise(float x, int rank, float persistence = 0.5f);

private:
	struct Octave
	{
		Octave(int rank, float firstValue, float lastValue);

		float value(float x) const;

		int rank;
		std::vector<Vector2> signalValues;
	};

	struct Interval
	{
		Interval(float min, float max, float firstValue);
		
		float lastValue() const { return octaves.front().signalValues.back().y; }
		void addOctave();

		float min;
		float max;
		std::vector<Octave> octaves;
	};

	std::vector<Interval> m_intervals;
};
