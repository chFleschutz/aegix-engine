#include "Scene.h"

#include "core/engine.h"
#include "graphics/static_mesh.h"
#include "graphics/systems/default_render_system.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/systems/camera_system.h"
#include "scripting/movement/kinematic_movement_controller.h"
#include "utils/math_utils.h"

#include <gltf.h>
#include <gltf_utils.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace Aegix::Scene
{
	Scene::Scene()
	{
		m_mainCamera = createEntity("Main Camera");
		m_mainCamera.addComponent<Camera>();
		m_mainCamera.addComponent<Scripting::KinematcMovementController>();
		m_mainCamera.component<Transform>() = Transform{
			.location = { 0.0f, -15.0f, 10.0f },
			.rotation = { glm::radians(-30.0f), 0.0f, 0.0f}
		};

		addSystem<CameraSystem>();
	}

	Entity Scene::createEntity(const std::string& name, const glm::vec3& location)
	{
		Entity entity = { m_registry.create(), this };
		entity.addComponent<Transform>(location);
		entity.addComponent<Name>(name.empty() ? "Entity" : name);
		return entity;
	}

	void Scene::destroyEntity(Entity entity)
	{
		// TODO: Destroy all scripts attached to the entity
		m_registry.destroy(entity);
	}

	void Scene::update(float deltaSeconds)
	{
		for (auto& system : m_systems)
		{
			system->onUpdate(deltaSeconds, *this);
		}

		m_scriptManager.update(deltaSeconds);
	}

	static auto loadTexture(GLTF::GLTF& gltf, size_t texureIndex, VkFormat format) -> std::shared_ptr<Graphics::Texture>
	{
		auto& source = gltf.textures[texureIndex].source;
		assert(source.has_value() && "Texture source is required");

		std::shared_ptr<Graphics::Texture> texture = nullptr;
		auto& image = gltf.images[source.value()];
		std::visit([&](auto&& data)
			{
				using T = std::decay_t<decltype(data)>;
				if constexpr (std::is_same_v<T, GLTF::Image::UriData>)
				{
					texture = Graphics::Texture::create(gltf.basePath / data.uri, format);
				}
				else if constexpr (std::is_same_v<T, GLTF::Image::BufferViewData>)
				{
					assert(false && "BufferView image data not supported");
				}
			}, image.data);

		return texture;
	}

	auto Scene::load(const std::filesystem::path& path) -> Entity
	{
		if (path.extension() == ".gltf" || path.extension() == ".glb")
		{
			return loadGLTF(path);
		}
		else if (path.extension() == ".obj")
		{
			auto entity = createEntity(path.stem().string());
			entity.addComponent<Mesh>(Graphics::StaticMesh::create(path));
			//entity.addComponent<Graphics::DefaultMaterial>(); // TODO: Add default material
			return entity;
		}
		else
		{
			assert(false && "Unsupported file format");
		}

		return Entity{};
	}

	auto Scene::loadGLTF(const std::filesystem::path& path) -> Entity
	{
		auto gltfResult = GLTF::load(path);
		assert(gltfResult.has_value() && "Failed to load GLTF file");
		auto& gltf = gltfResult.value();

		auto& renderer = Engine::instance().renderer();
		auto& device = Engine::instance().device();

		auto& scene = gltf.scenes[gltf.startScene.value_or(0)];

		// Default fallback material
		auto textureBlack = Graphics::Texture::create({ 1, 1 }, glm::vec4{ 0.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto textureWhite = Graphics::Texture::create({ 1, 1 }, glm::vec4{ 1.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto defaultNormal = Graphics::Texture::create({ 1, 1 }, glm::vec4{ 0.5f, 0.5f, 1.0f, 0.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto defaultMaterial = renderer.createMaterialInstance<Graphics::DefaultMaterial>(
			textureWhite, defaultNormal, textureWhite, textureBlack, textureBlack);

		// Load all meshes
		std::vector<std::vector<std::shared_ptr<Graphics::StaticMesh>>> meshMap(gltf.meshes.size());
		for (size_t i = 0; i < gltf.meshes.size(); i++)
		{
			auto& mesh = gltf.meshes[i];
			for (auto& primitive : mesh.primitives)
			{
				Graphics::StaticMesh::MeshInfo info{};
				GLTF::copyIndices(info.indices, primitive, gltf);
				GLTF::copyAttribute("POSITION", info.positions, primitive, gltf);
				GLTF::copyAttribute("COLOR_0", info.colors, primitive, gltf);
				GLTF::copyAttribute("NORMAL", info.normals, primitive, gltf);
				GLTF::copyAttribute("TEXCOORD_0", info.uvs, primitive, gltf);
				info.colors.resize(info.positions.size(), glm::vec3{ 1.0f });
				auto staticMesh = std::make_shared<Graphics::StaticMesh>(Engine::instance().device(), info);
				meshMap[i].emplace_back(staticMesh);
			}
		}

		// Load all materials
		std::vector<std::shared_ptr<Graphics::Texture>> textureMap(gltf.textures.size());
		std::vector<std::shared_ptr<Graphics::DefaultMaterialInstance>> materialMap;
		materialMap.reserve(gltf.materials.size());

		auto textureLoadLambda = [&](size_t textureIndex, VkFormat format)
			{
				if (textureMap[textureIndex])
					return textureMap[textureIndex];

				textureMap[textureIndex] = loadTexture(gltf, textureIndex, format);
				return textureMap[textureIndex];
			};

		for (auto& material : gltf.materials)
		{
			std::shared_ptr<Graphics::Texture> baseColorTexture = textureWhite;
			std::shared_ptr<Graphics::Texture> metallicRoughnessTexture = textureWhite;
			std::shared_ptr<Graphics::Texture> normalTexture = defaultNormal;
			std::shared_ptr<Graphics::Texture> occlusionTexture = textureBlack;
			std::shared_ptr<Graphics::Texture> emissiveTexture = textureBlack;

			if (material.pbrMetallicRoughness.has_value())
			{
				auto& pbr = material.pbrMetallicRoughness.value();
				if (pbr.baseColorTexture.has_value())
					baseColorTexture = textureLoadLambda(pbr.baseColorTexture->index, VK_FORMAT_R8G8B8A8_SRGB);

				if (pbr.metallicRoughnessTexture.has_value())
					metallicRoughnessTexture = textureLoadLambda(pbr.metallicRoughnessTexture->index, VK_FORMAT_R8G8B8A8_UNORM);
			}

			if (material.normalTexture.has_value())
				normalTexture = textureLoadLambda(material.normalTexture->index, VK_FORMAT_R8G8B8A8_UNORM);

			if (material.occlusionTexture.has_value())
				occlusionTexture = textureLoadLambda(material.occlusionTexture->index, VK_FORMAT_R8G8B8A8_UNORM);

			if (material.emissiveTexture.has_value())
				emissiveTexture = textureLoadLambda(material.emissiveTexture->index, VK_FORMAT_R8G8B8A8_SRGB);

			materialMap.emplace_back(renderer.createMaterialInstance<Graphics::DefaultMaterial>(
				baseColorTexture, normalTexture, metallicRoughnessTexture, occlusionTexture, emissiveTexture));
		}

		auto transformLambda = [](const GLTF::Node::Transform& transform) 
			{
				Transform result{};
				std::visit([&](auto&& val)
					{
						using T = std::decay_t<decltype(val)>;
						if constexpr (std::is_same_v<T, GLTF::Mat4>)
						{
							glm::mat4 matrix = glm::make_mat4(val.data());
							MathLib::decomposeTRS(matrix, result.location, result.rotation, result.scale);
						}
						else if constexpr (std::is_same_v<T, GLTF::Node::TRS>)
						{
							result.location = { val.translation[0], val.translation[1], val.translation[2] };
							result.rotation = glm::eulerAngles(glm::quat{ val.rotation[3], val.rotation[0], val.rotation[1], val.rotation[2] });
							result.scale = { val.scale[0], val.scale[1], val.scale[2] };
						}
					}, transform);
				return result;
			};

		for (auto nodeIndex : scene.nodes)
		{
			auto& node = gltf.nodes[nodeIndex];
			auto entity = createEntity(node.name.value_or("Node"));
			entity.component<Transform>() = transformLambda(node.transform);

			// Mesh
			if (!node.mesh.has_value())
				continue;

			size_t meshIndex = node.mesh.value();
			auto& mesh = gltf.meshes[meshIndex];
			for (size_t i = 0; i < mesh.primitives.size(); i++)
			{
				auto& primitive = mesh.primitives[i];
				auto& material = primitive.material.has_value() ? materialMap[*primitive.material] : defaultMaterial;
				
				auto meshEntity = createEntity(mesh.name.value_or("Mesh") + std::to_string(i));
				meshEntity.addComponent<Mesh>(meshMap[meshIndex][i]);
				meshEntity.addComponent<Graphics::DefaultMaterial>(material);
				meshEntity.component<Transform>() = Transform{
					.location = entity.component<Transform>().location,
					.rotation = { glm::radians(90.0f), 0.0f, 0.0f },
					.scale = { 0.04f, 0.04f, 0.04f }
				};
			}
		}

		// TODO: Return root entity
		return Entity{};
	}
}
