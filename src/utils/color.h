#pragma once

#include <glm/glm.hpp>

namespace Aegix
{
	class Color
	{
	public:
		static glm::vec4 red() { return glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f }; }
		static glm::vec4 green() { return glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f }; }
		static glm::vec4 blue() { return glm::vec4{ 0.0f, 0.0f, 1.0f, 1.0f }; }
		static glm::vec4 white() { return glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f }; }
		static glm::vec4 gray() { return glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f }; }
		static glm::vec4 black() { return glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }; }
		static glm::vec4 yellow() { return glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f }; }
		static glm::vec4 orange() { return glm::vec4{ 1.0f, 0.5f, 0.0f, 1.0f }; }
		static glm::vec4 purple() { return glm::vec4{ 0.5f, 0.0f, 1.0f, 1.0f }; }
		static glm::vec4 pink() { return glm::vec4{ 1.0f, 0.0f, 1.0f, 1.0f }; }
		static glm::vec4 brown() { return glm::vec4{ 0.5f, 0.25f, 0.0f, 1.0f }; }
		static glm::vec4 random();

		static glm::vec4 fromHex(const std::string& hex);
	};
}
