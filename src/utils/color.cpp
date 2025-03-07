#include "color.h"

#include "math/random.h"

#include <cassert>
#include <sstream>

namespace Aegix
{
	glm::vec4 Color::random()
	{
		return glm::vec4{ Random::uniformFloat(), Random::uniformFloat(), Random::uniformFloat(), 1.0f };
	}

	glm::vec4 Color::fromHex(const std::string& hex)
	{
		assert(hex.length() == 7 and hex[0] == '#' and "Invalid hex color format");

		if (hex.size() != 7 or hex[0] != '#')
			return glm::vec4{ 1.0f };

		unsigned int rgbValue;
		std::istringstream stream(hex.substr(1));
		stream >> std::hex >> rgbValue;

		int red = (rgbValue >> 16) & 0xFF;
		int green = (rgbValue >> 8) & 0xFF;
		int blue = rgbValue & 0xFF;

		return glm::vec4{ red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f };
	}
}
