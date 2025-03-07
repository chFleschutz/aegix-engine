#include "gltf_loader.h"

#include "core/engine.h"

#include <gltf_utils.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <cassert>
#include <variant>

namespace Aegix::Scene
{
	GLTFLoader::GLTFLoader(Scene& scene, const std::filesystem::path& path)
	{
		m_gltf = GLTF::load(path);
		assert(m_gltf.has_value() && "Failed to load GLTF file");

		if (m_gltf->scenes.empty())
			return;

		m_defaultBlack = Graphics::Texture::create({ 1, 1 }, glm::vec4{ 0.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		m_defaultWhite = Graphics::Texture::create({ 1, 1 }, glm::vec4{ 1.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		m_defaultNormal = Graphics::Texture::create({ 1, 1 }, glm::vec4{ 0.5f, 0.5f, 1.0f, 0.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		m_defaultMaterial = Engine::instance().renderer().createMaterialInstance<Graphics::DefaultMaterial>(
			m_defaultWhite, m_defaultNormal, m_defaultWhite, m_defaultBlack, m_defaultBlack);

		m_meshes.resize(m_gltf->meshes.size());
		m_materials.resize(m_gltf->materials.size());
		m_textures.resize(m_gltf->textures.size());
		m_entities.reserve(m_gltf->nodes.size() + m_gltf->materials.size()); // Rough estimate
		
		size_t startScene = m_gltf->startScene.value_or(0);
		std::string sceneName = m_gltf->scenes[startScene].name.value_or(path.stem().string());
		m_rootEntity = scene.createEntity(sceneName);
		
		loadScene(scene, startScene);
	}

	void GLTFLoader::loadScene(Scene& scene, size_t sceneIndex)
	{
		auto& sceneNode = m_gltf->scenes[sceneIndex];

		// Correct coordinate system (GLTF uses Y-up, Z-forward)
		m_rootEntity.component<Transform>().rotation = glm::vec3{ glm::radians(90.0f), 0.0f, 0.0f };

		struct Node
		{
			size_t index;
			Entity parent;
		};

		std::vector<Node> nodeStack;
		nodeStack.reserve(m_gltf->nodes.size());
		for (auto& nodeIndex : sceneNode.nodes)
			nodeStack.emplace_back(Node{ nodeIndex, m_rootEntity });

		uint32_t nodeCounter = 0;
		while (!nodeStack.empty())
		{
			auto& [nodeIndex, parent] = nodeStack.back();
			auto& node = m_gltf->nodes[nodeIndex];

			// Create entity for node
			auto nodeEntity = scene.createEntity(node.name.value_or("Node" + std::to_string(nodeCounter++)));
			nodeEntity.component<Transform>() = toTransform(node.transform);
			nodeEntity.setParent(parent);

			nodeStack.pop_back();

			for (auto& childIndex : node.children)
				nodeStack.emplace_back(Node{ childIndex, nodeEntity });
			
			// Mesh
			if (!node.mesh.has_value())
				continue;

			size_t meshIndex = node.mesh.value();
			auto& mesh = m_gltf->meshes[meshIndex];
			m_meshes[meshIndex].resize(mesh.primitives.size());
			for (size_t i = 0; i < mesh.primitives.size(); i++)
			{
				auto& primitive = mesh.primitives[i];
				auto material = primitive.material ? loadMaterial(*primitive.material) : m_defaultMaterial;

				auto meshEntity = scene.createEntity(mesh.name.value_or("Mesh") + std::to_string(i));
				meshEntity.addComponent<Graphics::DefaultMaterial>(material);
				meshEntity.addComponent<Mesh>(loadMesh(meshIndex, i));
				meshEntity.setParent(nodeEntity);
			}
		}
	}

	auto GLTFLoader::toTransform(const GLTF::Node::Transform& nodeTransform) -> Transform
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
			}, nodeTransform);
		return result;
	}

	auto GLTFLoader::loadMesh(size_t meshIndex, size_t primitiveIndex) -> std::shared_ptr<Graphics::StaticMesh>
	{
		assert(meshIndex < m_meshes.size() && "Mesh index is out of range");
		assert(primitiveIndex < m_gltf->meshes[meshIndex].primitives.size() && "Primitive index is out of range");

		// Return cached mesh
		if (m_meshes[meshIndex][primitiveIndex])
			return m_meshes[meshIndex][primitiveIndex];

		// Load mesh
		auto& primitive = m_gltf->meshes[meshIndex].primitives[primitiveIndex];
		Graphics::StaticMesh::MeshInfo info{};
		GLTF::copyIndices(info.indices, primitive, *m_gltf);
		GLTF::copyAttribute("POSITION", info.positions, primitive, *m_gltf);
		GLTF::copyAttribute("COLOR_0", info.colors, primitive, *m_gltf);
		GLTF::copyAttribute("NORMAL", info.normals, primitive, *m_gltf);
		GLTF::copyAttribute("TEXCOORD_0", info.uvs, primitive, *m_gltf);

		size_t vertexCount = info.positions.size();
		if (info.colors.size() != vertexCount)
			info.colors.resize(vertexCount, glm::vec3{ 1.0f });

		if (info.normals.size() != vertexCount)
			info.normals.resize(vertexCount, glm::vec3{ 0.0f, 0.0f, 1.0f });

		if (info.uvs.size() != vertexCount)
			info.uvs.resize(vertexCount, glm::vec2{ 0.0f });

		auto mesh = std::make_shared<Graphics::StaticMesh>(Engine::instance().device(), info);
		m_meshes[meshIndex].emplace_back(mesh);
		return mesh;
	}

	auto GLTFLoader::loadMaterial(size_t materialIndex) -> std::shared_ptr<Graphics::DefaultMaterialInstance>
	{
		assert(materialIndex < m_gltf->materials.size() && "Material index is out of range");

		// Return cached material
		if (m_materials[materialIndex])
			return m_materials[materialIndex];

		auto& material = m_gltf->materials[materialIndex];
		std::shared_ptr<Graphics::Texture> baseColorTexture = m_defaultWhite;
		std::shared_ptr<Graphics::Texture> normalTexture = m_defaultNormal;
		std::shared_ptr<Graphics::Texture> metallicRoughnessTexture = m_defaultWhite;
		std::shared_ptr<Graphics::Texture> occlusionTexture = m_defaultWhite;
		std::shared_ptr<Graphics::Texture> emissiveTexture = m_defaultBlack;

		if (auto& pbr = material.pbrMetallicRoughness)
		{
			if (auto& baseColor = pbr->baseColorTexture)
				baseColorTexture = loadTexture(baseColor->index, VK_FORMAT_R8G8B8A8_SRGB);

			if (auto& metallicRoughnes = pbr->metallicRoughnessTexture)
				metallicRoughnessTexture = loadTexture(metallicRoughnes->index, VK_FORMAT_R8G8B8A8_UNORM);
		}

		if (auto& normal = material.normalTexture)
			normalTexture = loadTexture(normal->index, VK_FORMAT_R8G8B8A8_UNORM);

		if (auto& occlusion = material.occlusionTexture)
			occlusionTexture = loadTexture(occlusion->index, VK_FORMAT_R8G8B8A8_UNORM);

		if (auto& emissive = material.emissiveTexture)
			emissiveTexture = loadTexture(emissive->index, VK_FORMAT_R8G8B8A8_SRGB);

		auto& renderer = Engine::instance().renderer();
		auto materialInstance = renderer.createMaterialInstance<Graphics::DefaultMaterial>(
			baseColorTexture, normalTexture, metallicRoughnessTexture, occlusionTexture, emissiveTexture);
		m_materials[materialIndex] = materialInstance;
		return materialInstance;
	}

	auto GLTFLoader::loadTexture(size_t textureIndex, VkFormat format) -> std::shared_ptr<Graphics::Texture>
	{
		assert(textureIndex < m_gltf->textures.size() && "Texture index is out of range");

		// Return cached texture
		if (m_textures[textureIndex])
			return m_textures[textureIndex];

		// Load texture
		auto& source = m_gltf->textures[textureIndex].source;
		assert(source.has_value() && "Texture source is required");

		auto& image = m_gltf->images[source.value()];
		std::shared_ptr<Graphics::Texture> texture;
		std::visit([&](auto&& val)
			{
				using T = std::decay_t<decltype(val)>;
				if constexpr (std::is_same_v<T, GLTF::Image::UriData>)
				{
					texture = Graphics::Texture::create(m_gltf->basePath / val.uri, format);
				}
				else if constexpr (std::is_same_v<T, GLTF::Image::BufferViewData>)
				{
					assert(false && "BufferView image data not supported yet");
				}
			}, image.data);

		m_textures[textureIndex] = texture;
		return texture;
	}
}
