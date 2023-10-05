#include <glm/glm.hpp>

#include <iostream>


// Alias for glm 

using Vector2 = glm::vec2;
using Vector3 = glm::vec3;
using Vector4 = glm::vec4;


// Overload the << operator for glm types for easy printing

std::ostream& operator<<(std::ostream& os, const Vector2& vector)
{
    return os << "{ " << vector.x << ", " << vector.y << " }";
}

std::ostream& operator<<(std::ostream& os, const Vector3& vector) 
{
    return os << "{ " << vector.x << ", " << vector.y << ", " << vector.z << " }";
}

std::ostream& operator<<(std::ostream& os, const Vector4& vector) 
{
    return os << "{ " << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << " }";
}