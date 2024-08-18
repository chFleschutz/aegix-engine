#pragma once

#include "utils/math_utils.h"

#include <string>

namespace Aegix
{
	class Color
	{
	public:
		static Vector4 red() { return Vector4{ 1.0f, 0.0f, 0.0f, 1.0f }; }
		static Vector4 green() { return Vector4{ 0.0f, 1.0f, 0.0f, 1.0f }; }
		static Vector4 blue() { return Vector4{ 0.0f, 0.0f, 1.0f, 1.0f }; }
		static Vector4 white() { return Vector4{ 1.0f, 1.0f, 1.0f, 1.0f }; }
		static Vector4 gray() { return Vector4{ 0.5f, 0.5f, 0.5f, 1.0f }; }
		static Vector4 black() { return Vector4{ 0.0f, 0.0f, 0.0f, 1.0f }; }
		static Vector4 yellow() { return Vector4{ 1.0f, 1.0f, 0.0f, 1.0f }; }
		static Vector4 orange() { return Vector4{ 1.0f, 0.5f, 0.0f, 1.0f }; }
		static Vector4 purple() { return Vector4{ 0.5f, 0.0f, 1.0f, 1.0f }; }
		static Vector4 pink() { return Vector4{ 1.0f, 0.0f, 1.0f, 1.0f }; }
		static Vector4 brown() { return Vector4{ 0.5f, 0.25f, 0.0f, 1.0f }; }
		static Vector4 random();

		static Vector4 fromHex(const std::string& hex);
	};
}
