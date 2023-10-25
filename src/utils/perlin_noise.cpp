#include "perlin_noise.h"

#include "utils/random.h"


PerlinNoise1D::PerlinNoise1D(int rank)
	: m_rank(rank)
{
	m_intervals.emplace_back(Interval(m_rank, 0.0f, 1.0f));
}

float PerlinNoise1D::noise(float x, float persistence)
{
	// Create new intervals if needed
	while (m_intervals.back().max < x)
	{
		addInterval();
	}

	// Get interval of x
	auto& interval = m_intervals[static_cast<int>(x)];
	auto relativeX = MathLib::percentage(x, interval.min, interval.max);

	// Calculate noise value
	float noiseValue = 0.0f;
	for (auto& octave : interval.octaves)
	{
		noiseValue += octave.value(relativeX) * static_cast<float>(std::pow(persistence, octave.rank()));
	}
	return noiseValue;
}

void PerlinNoise1D::addInterval()
{
	auto& lastInterval = m_intervals.back();
	m_intervals.emplace_back(Interval(m_rank, lastInterval));
}

PerlinNoise1D::Octave::Octave(int rank) : Octave(rank, Random::uniformFloat())
{
}

PerlinNoise1D::Octave::Octave(int rank, float firstValue)
{
	// Generate random signal values
	m_signalValues.emplace_back(Vector2{0.0f, firstValue});
	m_signalValues.emplace_back(Vector2{1.0f, Random::uniformFloat()});

	while (m_rank < rank)
	{
		for (int i = 1; i < m_signalValues.size(); i += 2)
		{
			float pos = (m_signalValues[i].x + m_signalValues[i - 1].x) / 2.0f;
			float value = Random::uniformFloat();
			m_signalValues.insert(m_signalValues.begin() + i, { pos, value });
		}
		m_rank++;
	}
}

float PerlinNoise1D::Octave::value(float x)
{
	// Binary search for the two closest signal values 
	size_t left = 0;
	size_t right = m_signalValues.size() - 1;
	while (left < right - 1)
	{
		size_t middle = (left + right) / 2;
		if (x < m_signalValues[middle].x)
		{
			right = middle;
		}
		else
		{
			left = middle;
		}
	}
	// Interpolate between the two closest signal values with tanh
	float percent = MathLib::percentage(x, m_signalValues[left].x, m_signalValues[right].x);
	return std::lerp(m_signalValues[left].y, m_signalValues[right].y, MathLib::tanh01(percent));
}

PerlinNoise1D::Interval::Interval(int rank, float minBounds, float maxBounds)
	: min(minBounds)
	, max(maxBounds)
{
	for (int i = 1; i <= rank; i++)
	{
		octaves.emplace_back(Octave(i));
	}
}

PerlinNoise1D::Interval::Interval(int rank, const Interval& previousInterval)
{
	min = previousInterval.max;
	max = previousInterval.max + previousInterval.max - previousInterval.min;

	for (const auto& octave : previousInterval.octaves)
	{
		octaves.emplace_back(Octave(octave.rank(), octave.lastValue()));
	}
}
