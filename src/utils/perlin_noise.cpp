#include "perlin_noise.h"

#include "utils/random.h"

#include <cassert>


PerlinNoise1D::PerlinNoise1D(float bandwidth)
	: m_bandwidth(bandwidth)
{
	addInterval(Random::uniformFloat());
}

float PerlinNoise1D::noise(float x, int rank, float persistence)
{
	assert(x >= 0.0f && "Perlin noise only works for positive values");

	int xIntervalIndex = static_cast<int>(x / m_bandwidth); // Get the interval of x
	float xRelative = x / m_bandwidth - xIntervalIndex;		// Transform x to the interval [0, 1]

	// Create new intervals if needed
	while (m_intervals.size() - 1 < xIntervalIndex)
	{
		auto& lastInterval = m_intervals.back();
		addInterval(lastInterval.lastValue());
	}

	// Create new octaves if needed
	auto& xInterval = m_intervals[xIntervalIndex];
	while (xInterval.octaves.size() <= rank)
	{
		addOctave(xIntervalIndex);
	}

	// Calculate noise value
	float noiseValue = 0.0f;
	for (const auto& octave : xInterval.octaves)
	{
		if (octave.rank > rank)
			break;

		noiseValue += octave.value(xRelative) * static_cast<float>(std::pow(persistence, octave.rank));
	}

	return noiseValue;
}

void PerlinNoise1D::addInterval(float firstValue)
{
	m_intervals.emplace_back(Interval(firstValue));
}

void PerlinNoise1D::addOctave(int intervalIndex)
{
	assert(intervalIndex >= 0 and intervalIndex < m_intervals.size() and "Can't add octave, because of invalid interval index");

	float firstValue = 0.0f;
	float lastValue = 0.0f;
	int rank = m_intervals[intervalIndex].octaves.back().rank + 1; 

	// Get the last value of the previous interval
	if (intervalIndex - 1 >= 0 and m_intervals[intervalIndex - 1].octaves.size() >= rank)
	{
		const auto& previousInterval = m_intervals[intervalIndex - 1];
		const auto& octave = previousInterval.octaves[rank - 1];
		firstValue = octave.signalValues.back().y;
	}
	else
	{
		firstValue = Random::uniformFloat();
	}

	// Get the first value of the next interval
	if (intervalIndex + 1 < m_intervals.size() and m_intervals[intervalIndex + 1].octaves.size() >= rank)
	{
		const auto& nextInterval = m_intervals[intervalIndex + 1];
		const auto& octavei = nextInterval.octaves[rank - 1];
		lastValue = octavei.signalValues.front().y;
	}
	else
	{
		lastValue = Random::uniformFloat();
	}

	// Add the octave
	m_intervals[intervalIndex].addOctave(firstValue, lastValue);
}


PerlinNoise1D::Octave::Octave(int rank, float firstValue, float lastValue)
	: rank(rank)
{
	signalValues.emplace_back(Vector2(0.0f, firstValue));
	signalValues.emplace_back(Vector2(1.0f, lastValue));

	for (int i = 1; i < rank; i++)
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

PerlinNoise1D::Interval::Interval(float firstValue)
{
	octaves.emplace_back(Octave(1, firstValue, Random::uniformFloat()));
}

void PerlinNoise1D::Interval::addOctave(float firstValue, float lastValue)
{
	const auto& lastOctave = octaves.back();
	octaves.emplace_back(Octave(lastOctave.rank + 1, firstValue, lastValue));
}
