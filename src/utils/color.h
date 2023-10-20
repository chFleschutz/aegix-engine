#pragma once

#include "utils/math_utils.h"

#include <string>


class Color
{
public:
	Color() = default;
	Color(float r, float g, float b, float a = 1.0f);
	Color(int r, int g, int b, int a = 255);
	Color(const std::string& hex);

	static Color red() { return Color(1.0f, 0.0f, 0.0f); }
	static Color green() { return Color(0.0f, 1.0f, 0.0f); }
	static Color blue() { return Color(0.0f, 0.0f, 1.0f); }
	static Color white() { return Color(1.0f, 1.0f, 1.0f); }
	static Color gray() { return Color(0.5f, 0.5f, 0.5f); }
	static Color black() { return Color(0.0f, 0.0f, 0.0f); }
	static Color yellow() { return Color(1.0f, 1.0f, 0.0f); }
	static Color orange() { return Color(1.0f, 0.5f, 0.0f); }
	static Color purple() { return Color(0.5f, 0.0f, 1.0f); }
	static Color pink() { return Color(1.0f, 0.0f, 1.0f); }
	static Color brown() { return Color(0.5f, 0.25f, 0.0f); }
	static Color random();

	static glm::vec3 parse(const std::string& hex);

	glm::vec3 rgb() const { return { m_color.r, m_color.g, m_color.b }; }
	float alpha() const { return m_color.a; }

	bool operator==(const Color& other) const;
	bool operator!=(const Color& other) const;

private:
	glm::vec4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
};
