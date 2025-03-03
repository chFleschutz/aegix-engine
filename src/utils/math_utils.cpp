#include "math_utils.h"

#include <glm/gtx/vector_angle.hpp>

#include <iostream>

// Overload the << operator for Vectors to make printing them easier
std::ostream& operator<<(std::ostream& os, const glm::vec2& vec)
{
	return os << "{ " << vec.x << ", " << vec.y << " }";;
}

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
	return os << "{ " << vec.x << ", " << vec.y << ", " << vec.z << " }";;
}

std::ostream& operator<<(std::ostream& os, const glm::vec4& vec)
{
	return os << "{ " << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << " }";;
}


namespace Aegix
{
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

	glm::mat4 MathLib::tranformationMatrix(const glm::vec3& location, const glm::vec3& rotation, const glm::vec3& scale)
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		return glm::mat4{
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

	glm::mat3 MathLib::normalMatrix(const glm::vec3& rotation, const glm::vec3& scale)
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 invScale = 1.0f / scale;

		return glm::mat3{
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

	void MathLib::decomposeTRS(const glm::mat4& matrix, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		translation = glm::vec3(matrix[3]);
		scale.x = glm::length(glm::vec3(matrix[0]));
		scale.y = glm::length(glm::vec3(matrix[1]));
		scale.z = glm::length(glm::vec3(matrix[2]));
		glm::mat3 rotationMat = glm::mat3(matrix);
		rotationMat[0] /= scale.x;
		rotationMat[1] /= scale.y;
		rotationMat[2] /= scale.z;
		rotation = glm::eulerAngles(glm::quat_cast(rotationMat));
	}

	glm::vec3 MathLib::forward(const glm::vec3& rotation)
	{
		const float sx = glm::sin(rotation.x);
		const float cx = glm::cos(rotation.x);
		const float sy = glm::sin(rotation.y);
		const float cy = glm::cos(rotation.y);
		const float cz = glm::cos(rotation.z);
		const float sz = glm::sin(rotation.z);

		return { sx * sy * cz - cx * sz, sx * sy * sz + cx * cz, sx * cy };
	}

	glm::vec3 MathLib::right(const glm::vec3& rotation)
	{
		const float sy = glm::sin(rotation.y);
		const float cy = glm::cos(rotation.y);
		const float cz = glm::cos(rotation.z);
		const float sz = glm::sin(rotation.z);

		return { cy * cz, cy * sz, -sy };
	}

	glm::vec3 MathLib::up(const glm::vec3& rotation)
	{
		const float sx = glm::sin(rotation.x);
		const float cx = glm::cos(rotation.x);
		const float sy = glm::sin(rotation.y);
		const float cy = glm::cos(rotation.y);
		const float cz = glm::cos(rotation.z);
		const float sz = glm::sin(rotation.z);

		return { cx * sy * cz + sx * sz, cx * sy * sz - sx * cz, cx * cy };
	}

	glm::vec3 MathLib::normalize(const glm::vec3& vec)
	{
		assert(vec.length() > 0.0f && "Can't normalize vector of length 0");
		return glm::normalize(vec);
	}

	bool MathLib::inFOV(const glm::vec3& viewDirection, const glm::vec3& targetDirection, float fov)
	{
		return glm::angle(viewDirection, targetDirection) < 0.5f * fov;
	}
}
