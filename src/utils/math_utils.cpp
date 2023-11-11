#include "math_utils.h"

#include <iostream>

// Overload the << operator for Vectors to make printing them easier
std::ostream& operator<<(std::ostream& os, const Vector2& vec) 
{
	return os << "{ " << vec.x << ", " << vec.y  << " }";;
}

std::ostream& operator<<(std::ostream& os, const Vector3& vec)
{
	return os << "{ " << vec.x << ", " << vec.y << ", " << vec.z << " }";;
}

std::ostream& operator<<(std::ostream& os, const Vector4& vec)
{
	return os << "{ " << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << " }";;
}


float MathLib::percentage(float value, float min, float max)
{
	return (value - min) / (max - min);
}

float MathLib::sigmoid01(float x)
{
	return 1.0f / (1.0f + std::exp(-12.0f * x + 6.0f));
}

float MathLib::fastSigmoid01(float x)
{
	x = 12.0f * x - 6.0f;
	return x / (1.0f + std::abs(x)) / 2.0f + 0.5f;
}

float MathLib::tanh01(float x)
{
	return std::tanh(6.0f * x - 3.0f) / 2.0f + 0.5f;
}

Matrix4 MathLib::tranformationMatrix(const Vector3& location, const Vector3& rotation, const Vector3& scale)
{
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	return Matrix4{
		{
			scale.x * (c1 * c3 + s1 * s2 * s3),
			scale.x * (c2 * s3),
			scale.x * (c1 * s2 * s3 - c3 * s1),
			0.0f,
		},
		{
			scale.y * (c3 * s1 * s2 - c1 * s3),
			scale.y * (c2 * c3),
			scale.y * (c1 * c3 * s2 + s1 * s3),
			0.0f,
		},
		{
			scale.z * (c2 * s1),
			scale.z * (-s2),
			scale.z * (c1 * c2),
			0.0f,
		},
		{
			location.x, location.y, location.z, 1.0f
		} };
}

Matrix3 MathLib::normalMatrix(const Vector3& rotation, const Vector3& scale)
{
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const Vector3 invScale = 1.0f / scale;

	return Matrix3{
		{
			invScale.x * (c1 * c3 + s1 * s2 * s3),
			invScale.x * (c2 * s3),
			invScale.x * (c1 * s2 * s3 - c3 * s1),
		},
		{
			invScale.y * (c3 * s1 * s2 - c1 * s3),
			invScale.y * (c2 * c3),
			invScale.y * (c1 * c3 * s2 + s1 * s3),
		},
		{
			invScale.z * (c2 * s1),
			invScale.z * (-s2),
			invScale.z * (c1 * c2),
		} };
}

Vector3 MathLib::forward(const Vector3& rotation)
{
	const float sx = glm::sin(rotation.x);
	const float cx = glm::cos(rotation.x);
	const float sy = glm::sin(rotation.y);
	const float cy = glm::cos(rotation.y);
	const float cz = glm::cos(rotation.z);
	const float sz = glm::sin(rotation.z);

	return { cx * cz * sy + sx * sz, -cz * sx + cx * sy * sz, cx * cy };
}

Vector3 MathLib::right(const Vector3& rotation)
{
	const float sy = glm::sin(rotation.y);
	const float cy = glm::cos(rotation.y);
	const float cz = glm::cos(rotation.z);
	const float sz = glm::sin(rotation.z);

	return { cy * cz, cy * sz, -sy };
}

Vector3 MathLib::up(const Vector3& rotation)
{
	const float sx = glm::sin(rotation.x);
	const float cx = glm::cos(rotation.x);
	const float sy = glm::sin(rotation.y);
	const float cy = glm::cos(rotation.y);
	const float cz = glm::cos(rotation.z);
	const float sz = glm::sin(rotation.z);

	return { -cz * sx * sy + cx * sz, -cx * cz - sx * sy * sz, -cy * sx };
}
