#include "color.h"

#include "utils/random.h"

#include <cassert>
#include <sstream>

namespace Aegix
{
	Color::Color(float r, float g, float b, float a)
		: m_color{ r, g, b, a }
	{
	}

	Color::Color(int r, int g, int b, int a)
		: m_color{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }
	{
	}

	Color::Color(const std::string& hex)
		: m_color{ parse(hex), 1.0f }
	{
	}

	Color Color::random()
	{
		return Color(Random::uniformFloat(), Random::uniformFloat(), Random::uniformFloat());
	}

	Vector3 Color::parse(const std::string& hex)
	{
		assert(hex.length() == 7 and hex[0] == '#' and "Invalid hex color format");

		if (hex.size() != 7 or hex[0] != '#')
			return { 1.0f, 1.0f, 1.0f };

		unsigned int rgbValue;
		std::istringstream stream(hex.substr(1));
		stream >> std::hex >> rgbValue;

		int red = (rgbValue >> 16) & 0xFF;
		int green = (rgbValue >> 8) & 0xFF;
		int blue = rgbValue & 0xFF;

		return { red / 255.0f, green / 255.0f, blue / 255.0f };
	}

	bool Color::operator==(const Color& other) const
	{
		return m_color == other.m_color;
	}

	bool Color::operator!=(const Color& other) const
	{
		return m_color != other.m_color;
	}
}
