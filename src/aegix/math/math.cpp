#include "pch.h"
#include "math.h"

namespace Aegix::Math
{
	auto forward(const glm::vec3& rotation) -> glm::vec3
	{
		const float sx = glm::sin(rotation.x);
		const float cx = glm::cos(rotation.x);
		const float sy = glm::sin(rotation.y);
		const float cy = glm::cos(rotation.y);
		const float cz = glm::cos(rotation.z);
		const float sz = glm::sin(rotation.z);
		return { sx * sy * cz - cx * sz, sx * sy * sz + cx * cz, sx * cy };
	}

	auto forward(const glm::quat& rotation) -> glm::vec3
	{
		return rotation * World::FORWARD;
	}

	auto right(const glm::vec3& rotation) -> glm::vec3
	{
		const float sy = glm::sin(rotation.y);
		const float cy = glm::cos(rotation.y);
		const float cz = glm::cos(rotation.z);
		const float sz = glm::sin(rotation.z);
		return { cy * cz, cy * sz, -sy };
	}

	auto right(const glm::quat& rotation) -> glm::vec3
	{
		return rotation * World::RIGHT;
	}

	auto up(const glm::vec3& rotation) -> glm::vec3
	{
		const float sx = glm::sin(rotation.x);
		const float cx = glm::cos(rotation.x);
		const float sy = glm::sin(rotation.y);
		const float cy = glm::cos(rotation.y);
		const float cz = glm::cos(rotation.z);
		const float sz = glm::sin(rotation.z);
		return { cx * sy * cz + sx * sz, cx * sy * sz - sx * cz, cx * cy };
	}

	auto up(const glm::quat& rotation) -> glm::vec3
	{
		return rotation * World::UP;
	}

	auto tranformationMatrix(const glm::vec3& location, const glm::vec3& rotation, const glm::vec3& scale) -> glm::mat4
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

	auto tranformationMatrix(const glm::vec3& location, const glm::quat& rotation, const glm::vec3& scale) -> glm::mat4
	{
		glm::mat3 rotMat = glm::mat3_cast(rotation);
		rotMat[0] *= scale.x;
		rotMat[1] *= scale.y;
		rotMat[2] *= scale.z;

		return glm::mat4{
			glm::vec4{ rotMat[0], 0.0f },
			glm::vec4{ rotMat[1], 0.0f },
			glm::vec4{ rotMat[2], 0.0f },
			glm::vec4{ location, 1.0f }
		};
	}

	auto normalMatrix(const glm::vec3& rotation, const glm::vec3& scale) -> glm::mat3
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

	auto normalMatrix(const glm::quat& rotation, const glm::vec3& scale) -> glm::mat3
	{
		glm::mat3 rotMat = glm::mat3_cast(rotation);
		rotMat[0] *= 1.0f / scale.x;
		rotMat[1] *= 1.0f / scale.y;
		rotMat[2] *= 1.0f / scale.z;
		return rotMat;
	}

	void decomposeTRS(const glm::mat4& matrix, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
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

	void decomposeTRS(const glm::mat4& matrix, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale)
	{
		translation = glm::vec3(matrix[3]);
		scale.x = glm::length(glm::vec3(matrix[0]));
		scale.y = glm::length(glm::vec3(matrix[1]));
		scale.z = glm::length(glm::vec3(matrix[2]));
		glm::mat3 rotationMat = glm::mat3(matrix);
		rotationMat[0] /= scale.x;
		rotationMat[1] /= scale.y;
		rotationMat[2] /= scale.z;
		rotation = glm::quat_cast(rotationMat);
	}

	auto percentage(float value, float min, float max) -> float
	{
		return (value - min) / (max - min);
	}

	auto inFOV(const glm::vec3& viewDirection, const glm::vec3& targetDirection, float fov) -> bool
	{
		return glm::angle(viewDirection, targetDirection) < 0.5f * fov;
	}
}
