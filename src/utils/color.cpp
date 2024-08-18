#include "color.h"

#include "utils/random.h"

#include <cassert>
#include <sstream>

namespace Aegix
{
	Vector4 Color::random()
	{
		return Vector4{ Random::uniformFloat(), Random::uniformFloat(), Random::uniformFloat(), 1.0f };
	}

	Vector4 Color::fromHex(const std::string& hex)
	{
		assert(hex.length() == 7 and hex[0] == '#' and "Invalid hex color format");

		if (hex.size() != 7 or hex[0] != '#')
			return Vector4{ 1.0f };

		unsigned int rgbValue;
		std::istringstream stream(hex.substr(1));
		stream >> std::hex >> rgbValue;

		int red = (rgbValue >> 16) & 0xFF;
		int green = (rgbValue >> 8) & 0xFF;
		int blue = rgbValue & 0xFF;

		return Vector4{ red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f };
	}
}
