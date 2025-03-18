#pragma once 

#include <glm/glm.hpp>	
#include <glm/gtc/quaternion.hpp>

// Overloads to print glm types

auto operator<<(std::ostream& os, const glm::vec2& vec) -> std::ostream&
{
	return os << "{ " << vec.x << ", " << vec.y << " }";
}

auto operator<<(std::ostream& os, const glm::vec3& vec) -> std::ostream&
{
	return os << "{ " << vec.x << ", " << vec.y << ", " << vec.z << " }";
}

auto operator<<(std::ostream& os, const glm::vec4& vec) -> std::ostream&
{
	return os << "{ " << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << " }";
}

auto operator<<(std::ostream& os, const glm::quat& quat) -> std::ostream&
{
	return os << "{ " << quat.x << ", " << quat.y << ", " << quat.z << ", " << quat.w << " }";
}

auto operator<<(std::ostream& os, const glm::mat4& mat) -> std::ostream&
{
	os << "{ ";
	for (int i = 0; i < 4; i++)
	{
		os << "{ ";
		for (int j = 0; j < 4; j++)
		{
			os << mat[i][j];
			if (j < 3)
				os << ", ";
		}
		os << " }";
		if (i < 3)
			os << ", ";
	}
	os << " }";
	return os;
}
