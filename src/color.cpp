#include "color.h"

namespace vre
{
	const Color Color::white = Color(1.0f, 1.0f, 1.0f);
	const Color Color::black = Color(0.0f, 0.0f, 0.0f);
	const Color Color::red = Color(1.0f, 0.0f, 0.0f);
	const Color Color::green = Color(0.0f, 1.0f, 0.0f);
	const Color Color::blue = Color(0.0f, 0.0f, 1.0f);

	Color::Color() : r{1.0f}, g{1.0f}, b{1.0f}
	{
	}

	Color::Color(float red, float green, float blue) : r{red}, g{green}, b{blue}
	{
	}

	std::ostream& operator<<(std::ostream& os, const Color& color)
	{
		return os << "{ " << color.r << ", " << color.g << ", " << color.b << " }";
	}

} // namespace vre