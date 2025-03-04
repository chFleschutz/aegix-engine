#pragma once

#include "graphics/static_mesh.h"
#include "scene/entity.h"
#include "utils/math_utils.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>

namespace Aegix::Scripting
{
	class ScriptBase;
}

namespace Aegix
{
	/// @brief Gives a name to the entity
	struct Name
	{
		std::string name = "Entity";
	};

	/// @brief Stores the local transformation of the entity
	struct Transform
	{
		glm::vec3 location = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		auto forward() const -> glm::vec3 { return MathLib::forward(rotation); }
		auto right() const -> glm::vec3 { return MathLib::right(rotation); }
		auto up() const -> glm::vec3 { return MathLib::up(rotation); }
		auto matrix() const -> glm::mat4 { return MathLib::tranformationMatrix(location, rotation, scale); }
	};

	/// @brief Transformation of the entity in world space (including parent transforms)
	struct GlobalTransform
	{
		glm::vec3 location = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		auto forward() const -> glm::vec3 { return MathLib::forward(rotation); }
		auto right() const -> glm::vec3 { return MathLib::right(rotation); }
		auto up() const -> glm::vec3 { return MathLib::up(rotation); }
		auto matrix() const -> glm::mat4 { return MathLib::tranformationMatrix(location, rotation, scale); }
	};

	/// @brief Stores the parent entity
	struct Parent
	{
		Scene::Entity entity{};
	};

	/// @brief Stores the children entities as the first and last child of a linked list
	struct Children
	{
		Scene::Entity first{};
		Scene::Entity last{};
		size_t count = 0;
	};

	/// @brief Stores the next and previous sibling of the entity (for linked list)
	struct Siblings
	{
		Scene::Entity next{};
		Scene::Entity prev{};
	};

	/// @brief Holds a pointer to a static mesh
	struct Mesh
	{
		std::shared_ptr<Graphics::StaticMesh> staticMesh;
	};

	struct AmbientLight
	{
		glm::vec3 color = { 1.0f, 1.0f, 1.0f };
		float intensity = 0.1f;
	};

	struct DirectionalLight
	{
		glm::vec3 color = { 1.0f, 1.0f, 1.0f };
		float intensity = 1.0f;
	};

	/// @brief Creates a light 
	struct PointLight
	{
		glm::vec3 color = { 1.0f, 1.0f, 1.0f };
		float intensity = 100.0f;
	};

	/// @brief Holds a camera to view the scene
	struct Camera
	{
		float fov = glm::radians(45.0f);
		float aspect = 1.0f;
		float near = 0.1f;
		float far = 100.0f;

		glm::mat4 viewMatrix = glm::mat4{ 1.0f };
		glm::mat4 inverseViewMatrix = glm::mat4{ 1.0f };
		glm::mat4 projectionMatrix = glm::mat4{ 1.0f };
	};
}