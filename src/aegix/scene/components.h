#pragma once

#include "graphics/resources/material_instance.h"
#include "graphics/resources/static_mesh.h"
#include "graphics/resources/texture.h"
#include "math/math.h"
#include "scene/entity.h"

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
		glm::vec3 location{ 0.0f };
		glm::quat rotation{};
		glm::vec3 scale{ 1.0f };

		auto forward() const -> glm::vec3 { return Math::forward(rotation); }
		auto right() const -> glm::vec3 { return Math::right(rotation); }
		auto up() const -> glm::vec3 { return Math::up(rotation); }
		auto matrix() const -> glm::mat4 { return Math::tranformationMatrix(location, rotation, scale); }
	};

	/// @brief Transformation of the entity in world space (including parent transforms)
	struct GlobalTransform
	{
		glm::vec3 location{ 0.0f };
		glm::quat rotation{};
		glm::vec3 scale{ 1.0f };

		auto forward() const -> glm::vec3 { return Math::forward(rotation); }
		auto right() const -> glm::vec3 { return Math::right(rotation); }
		auto up() const -> glm::vec3 { return Math::up(rotation); }
		auto matrix() const -> glm::mat4 { return Math::tranformationMatrix(location, rotation, scale); }
	};

	/// @brief Stores the parent entity
	/// @note Use Entity::setParent to set the parent of an entity 
	struct Parent
	{
		Scene::Entity entity{};
	};

	/// @brief Stores the next and previous sibling of the entity (for linked list)
	/// @note Use Entity::addChild to add a child to an entity (siblings are automatically updated)
	struct Siblings
	{
		Scene::Entity next{};
		Scene::Entity prev{};
	};

	/// @brief Stores the children entities as the first and last child of a linked list
	/// @note Use Entity::addChild to add a child to an entity
	struct Children
	{
		struct Iterator
		{
			Scene::Entity current{};

			auto operator*() const -> Scene::Entity { return current; }
			auto operator++() -> Iterator& { current = current.get<Siblings>().next; return *this; }
			auto operator==(const Iterator& other) const -> bool { return current == other.current; }
		};

		struct ReverseIterator
		{
			Scene::Entity current{};

			auto operator*() const -> Scene::Entity { return current; }
			auto operator++() -> ReverseIterator& { current = current.get<Siblings>().prev; return *this; }
			auto operator==(const ReverseIterator& other) const -> bool { return current == other.current; };
		};

		size_t count = 0;
		Scene::Entity first{};
		Scene::Entity last{};

		auto begin() const -> Children::Iterator { return { first }; }
		auto end() const -> Children::Iterator { return { Scene::Entity{} }; }
		auto rbegin() const -> Children::ReverseIterator { return { last }; }
		auto rend() const -> Children::ReverseIterator { return { Scene::Entity{} }; }
	};

	struct Mesh
	{
		std::shared_ptr<Graphics::StaticMesh> staticMesh;
	};

	struct Material
	{
		std::shared_ptr<Graphics::MaterialInstance> instance;
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

	struct Environment
	{
		std::shared_ptr<Graphics::Texture> skybox;
		std::shared_ptr<Graphics::Texture> irradiance;
		std::shared_ptr<Graphics::Texture> prefiltered;
		std::shared_ptr<Graphics::Texture> brdfLUT;
	};


	template<typename T, typename... Ts>
	concept IsAnyOf = (std::is_same_v<T, Ts> || ...);

	/// @brief Defines all required components for an entity (Each entity MUST have these components)
	template<typename T>
	concept RequiredComponent = IsAnyOf<T, Name, Transform, GlobalTransform, Parent, Siblings, Children>;

	/// @brief Defines all optional components for an entity (Each entity MAY have these components)
	template<typename T>
	concept OptionalComponent = !RequiredComponent<T>;
}