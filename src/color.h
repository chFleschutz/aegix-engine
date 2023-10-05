#pragma once

#include <glm/glm.hpp>

#include <iostream>

namespace vre
{
	class Color : public glm::vec3
	{
	public:
		Color(float red, float green, float blue);

		static const Color white;
		static const Color black;
		static const Color red;
		static const Color green;
		static const Color blue;

		friend std::ostream& operator<<(std::ostream& os, const Color& color);

		float r, g, b;
	};

} // namespace vre
