#include "perlin_noise.h"

#include "math/interpolation.h"
#include "utils/random.h"

#include <cassert>

namespace Aegix
{
	PerlinNoise1D::PerlinNoise1D(float bandwidth)
		: m_bandwidth(bandwidth)
	{
		// Add the first interval
		m_intervals.emplace_back(Interval(Random::uniformFloat()));
	}

	float PerlinNoise1D::noise(float x, int rank, float persistence)
	{
		assert(x >= 0.0f && "Perlin noise only works for positive values");

		int xIntervalIndex = static_cast<int>(x / m_bandwidth); // Get the interval of x
		float xRelative = x / m_bandwidth - xIntervalIndex;		// Transform x to the interval [0, 1)

		// Create new intervals if needed
		while (m_intervals.size() - 1 < xIntervalIndex)
		{
			addInterval();
		}

		// Create new octaves if needed
		auto& xInterval = m_intervals[xIntervalIndex];
		while (xInterval.octaves.size() <= rank)
		{
			addOctave(xIntervalIndex);
		}

		// Calculate noise value for all octaves until rank
		float noiseValue = 0.0f;
		float maxAmplitude = 0.0f;
		for (const auto& octave : xInterval.octaves)
		{
			if (octave.rank > rank)
				break;

			float amplitude = static_cast<float>(std::pow(persistence, octave.rank));
			noiseValue += octave.value(xRelative) * amplitude;
			maxAmplitude += amplitude;
		}

		// Return normalized noise value
		return noiseValue / maxAmplitude;
	}

	void PerlinNoise1D::addInterval()
	{
		// Add new interval with the last value of the previous interval as its first value
		const auto& previousFirstOctave = m_intervals.back().octaves.front();
		m_intervals.emplace_back(Interval(previousFirstOctave.signalValues.back()));
	}

	void PerlinNoise1D::addOctave(int intervalIndex)
	{
		assert(intervalIndex >= 0 and intervalIndex < m_intervals.size() and "Can't add octave, because of invalid interval index");

		float firstValue = 0.0f;
		float lastValue = 0.0f;
		int rank = m_intervals[intervalIndex].octaves.back().rank + 1;

		// Get the last value of the previous interval if possible
		if (intervalIndex - 1 >= 0 and m_intervals[intervalIndex - 1].octaves.size() >= rank)
		{
			const auto& octave = m_intervals[intervalIndex - 1].octaves[rank - 1];
			firstValue = octave.signalValues.back();
		}
		else
		{
			firstValue = Random::uniformFloat();
		}

		// Get the first value of the next interval if possible
		if (intervalIndex + 1 < m_intervals.size() and m_intervals[intervalIndex + 1].octaves.size() >= rank)
		{
			const auto& octavei = m_intervals[intervalIndex + 1].octaves[rank - 1];
			lastValue = octavei.signalValues.front();
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
		signalValues.emplace_back(firstValue);
		signalValues.emplace_back(lastValue);
		// Add all random signal values for the rank
		for (int i = 1; i < rank; i++)
		{
			for (int i = 1; i < signalValues.size(); i += 2)
			{
				signalValues.insert(signalValues.begin() + i, Random::uniformFloat());
			}
		}
	}

	float PerlinNoise1D::Octave::value(float x) const
	{
		assert(x >= 0.0f and x < 1.0f and "x must be in the interval [0, 1)");

		// Find the two signal values that are closest to x
		int leftIndex = static_cast<int>((signalValues.size() - 1) * x);
		int rightIndex = leftIndex + 1;

		// Calculate the percentage where x is between the two signal values
		float chunkSize = 1.0f / (signalValues.size() - 1);
		float percent = Math::percentage(x, leftIndex * chunkSize, rightIndex * chunkSize);

		// Return the interpolated value
		return std::lerp(signalValues[leftIndex], signalValues[rightIndex], Math::tanh01(percent));
	}

	PerlinNoise1D::Interval::Interval(float firstValue)
	{
		// Add the first octave
		octaves.emplace_back(Octave(1, firstValue, Random::uniformFloat()));
	}

	void PerlinNoise1D::Interval::addOctave(float firstValue, float lastValue)
	{
		// Adds an octave with the rank of the last octave + 1
		octaves.emplace_back(Octave(octaves.back().rank + 1, firstValue, lastValue));
	}
}
