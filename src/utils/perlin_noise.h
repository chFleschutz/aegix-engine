#pragma once

#include "utils/math_utils.h"

#include <vector>

/// @brief 1D Perlin noise generator.
class PerlinNoise1D
{
public:
	explicit PerlinNoise1D(int rank = 2);

	/// @brief Returns the noise value at the given position.
	/// @param x Position.
	/// @param persistence Persistence of the noise.
	float noise(float x, float persistence = 0.5f);

private:
	class Octave
	{
	public:
		Octave(int rank);
		Octave(int rank, float firstValue);

		float value(float x);
		int rank() const { return m_rank; }
		float lastValue() const { return m_signalValues.back().y; }

	private:
		std::vector<Vector2> m_signalValues;
		int m_rank = 1;
	};

	struct Interval
	{
		Interval(int rank, float min, float max);
		Interval(int rank, const Interval& previousInterval);

		float min;
		float max;
		std::vector<Octave> octaves;
	};

	void addInterval();

	std::vector<Interval> m_intervals;
	int m_rank;
};
