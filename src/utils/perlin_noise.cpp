#include "perlin_noise.h"

#include "utils/random.h"

#include <cassert>


PerlinNoise1D::PerlinNoise1D()
{
	m_intervals.emplace_back(Interval(0.0f, 1.0f, Random::uniformFloat()));
}

float PerlinNoise1D::noise(float x, int rank, float persistence)
{
	assert(x >= 0.0f && "Perlin noise only works for positive values");

	// Create new intervals if needed
	while (m_intervals.back().max < x)
	{
		auto& lastInterval = m_intervals.back();
		m_intervals.emplace_back(Interval(lastInterval.max, lastInterval.max + 1.0f, lastInterval.lastValue()));
	}

	// Create new octaves if needed
	auto& xInterval = m_intervals[static_cast<int>(x)];
	while (xInterval.octaves.size() < rank)
	{
		xInterval.addOctave();
	}

	// Calculate noise value
	float noiseValue = 0.0f;
	float relativeX = MathLib::percentage(x, xInterval.min, xInterval.max);
	for (const auto& octave : xInterval.octaves)
	{
		if (octave.rank > rank)
			break;

		noiseValue += octave.value(relativeX) * static_cast<float>(std::pow(persistence, octave.rank));
	}

	return noiseValue;
}


PerlinNoise1D::Octave::Octave(int rank, float firstValue, float lastValue)
	: rank(rank)
{
	signalValues.emplace_back(Vector2(0.0f, firstValue));
	signalValues.emplace_back(Vector2(1.0f, lastValue));

	for (int i = 0; i < rank; i++)
	{
		for (int i = 1; i < signalValues.size(); i += 2)
		{
			float pos = (signalValues[i].x + signalValues[i - 1].x) / 2.0f;
			float value = Random::uniformFloat();
			signalValues.insert(signalValues.begin() + i, { pos, value });
		}
	}
}

float PerlinNoise1D::Octave::value(float x) const
{
	// Binary search for the two closest signal values to x
	size_t left = 0;
	size_t right = signalValues.size() - 1;
	while (left < right - 1)
	{
		size_t middle = (left + right) / 2;
		if (x < signalValues[middle].x)
		{
			right = middle;
		}
		else
		{
			left = middle;
		}
	}

	// Interpolate between the two closest signal values with tanh
	float percent = MathLib::percentage(x, signalValues[left].x, signalValues[right].x);
	return std::lerp(signalValues[left].y, signalValues[right].y, MathLib::tanh01(percent));
}

PerlinNoise1D::Interval::Interval(float minBounds, float maxBounds, float firstValue)
	: min(minBounds)
	, max(maxBounds)
{
	octaves.emplace_back(Octave(1, firstValue, Random::uniformFloat()));
}

void PerlinNoise1D::Interval::addOctave()
{
	const auto& lastOctave = octaves.back();
	octaves.emplace_back(Octave(lastOctave.rank + 1, lastOctave.signalValues.front().y, lastOctave.signalValues.back().y));
}
