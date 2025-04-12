#include "pch.h"

#include "static_mesh.h"

#include "engine.h"
#include "utils/file.h"

#include "gltf.h"
#include "gltf_utils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace Aegix::Graphics
{
	StaticMesh::StaticMesh(VulkanDevice& device, const StaticMesh::MeshInfo& info) : m_device{ device }
	{
		// Create vertex buffers (Order: Position, Color, Normal, UV)
		m_vertexCount = static_cast<uint32_t>(info.positions.size());
		AGX_ASSERT_X(info.colors.size() == m_vertexCount && info.normals.size() == m_vertexCount && info.uvs.size() == m_vertexCount, 
			"Vertex attribute count has to match");

		createVertexAttributeBuffer(info.positions);
		createVertexAttributeBuffer(info.colors);
		createVertexAttributeBuffer(info.normals);
		createVertexAttributeBuffer(info.uvs);

		createIndexBuffers(info.indices);
	}

	std::vector<VkVertexInputBindingDescription> StaticMesh::defaultBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(4);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(glm::vec3);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescriptions[1].binding = 1;
		bindingDescriptions[1].stride = sizeof(glm::vec3);
		bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescriptions[2].binding = 2;
		bindingDescriptions[2].stride = sizeof(glm::vec3);
		bindingDescriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescriptions[3].binding = 3;
		bindingDescriptions[3].stride = sizeof(glm::vec2);
		bindingDescriptions[3].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> StaticMesh::defaultAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = 0;
		attributeDescriptions[1].binding = 1;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = 0;
		attributeDescriptions[2].binding = 2;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = 0;
		attributeDescriptions[3].binding = 3;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = 0;
		return attributeDescriptions;
	}

	std::shared_ptr<StaticMesh> StaticMesh::create(const std::filesystem::path& filepath)
	{
		MeshInfo info{};
		
		if (filepath.extension() == ".gltf" || filepath.extension() == ".glb")
		{
			info.loadGLTF(filepath);
		}
		else if (filepath.extension() == ".obj")
		{
			info.loadOBJ(filepath);
		}
		else
		{
			AGX_ASSERT_X(false, "Unsupported file format");
		}

		return std::make_shared<StaticMesh>(Engine::instance().device(), info);
	}

	void StaticMesh::bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindVertexBuffers(commandBuffer, 0, m_vkBuffers.size(), m_vkBuffers.data(), m_bufferOffsets.data());

		if (m_indexBuffer)
			vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->buffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void StaticMesh::draw(VkCommandBuffer commandBuffer)
	{
		if (m_indexBuffer)
		{
			vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
		}
	}

	void StaticMesh::createIndexBuffers(const std::vector<uint32_t>& indices)
	{
		m_indexCount = static_cast<uint32_t>(indices.size());
		if (m_indexCount <= 0)
			return;

		VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;
		m_indexBuffer = std::make_unique<Buffer>(bufferSize, 1,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		m_indexBuffer->upload(indices.data(), bufferSize);
	}

	void StaticMesh::MeshInfo::loadOBJ(const std::filesystem::path& filepath)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.string().c_str());
		AGX_ASSERT_X(result, "Failed to load OBJ file");

		positions.clear();
		colors.clear();
		normals.clear();
		uvs.clear();
		indices.clear();

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				glm::vec3 position{};
				glm::vec3 color{};
				glm::vec3 normal{};
				glm::vec2 uv{};

				if (index.vertex_index >= 0)
				{
					position.x = attrib.vertices[3 * index.vertex_index + 0];
					position.y = attrib.vertices[3 * index.vertex_index + 1];
					position.z = attrib.vertices[3 * index.vertex_index + 2];

					color.r = attrib.colors[3 * index.vertex_index + 0];
					color.g = attrib.colors[3 * index.vertex_index + 1];
					color.b = attrib.colors[3 * index.vertex_index + 2];
				}

				if (index.normal_index >= 0)
				{
					normal.x = attrib.normals[3 * index.normal_index + 0];
					normal.y = attrib.normals[3 * index.normal_index + 1];
					normal.z = attrib.normals[3 * index.normal_index + 2];
				}

				if (index.texcoord_index >= 0)
				{
					uv.x = attrib.texcoords[2 * index.texcoord_index + 0];
					uv.y = attrib.texcoords[2 * index.texcoord_index + 1];
				}

				positions.emplace_back(position);
				colors.emplace_back(color);
				normals.emplace_back(normal);
				uvs.emplace_back(uv);
			}
		}
	}

	void StaticMesh::MeshInfo::loadGLTF(const std::filesystem::path& filepath)
	{
		auto gltf = GLTF::load(filepath);
		AGX_ASSERT_X(gltf.has_value(), "Failed to load GLTF file");

		// TODO: Support multiple meshes and primitives
		auto& primitive = gltf->meshes[0].primitives[0];

		GLTF::copyIndices(indices, primitive, *gltf);
		GLTF::copyAttribute("POSITION", positions, primitive, *gltf);
		GLTF::copyAttribute("COLOR_0", colors, primitive, *gltf);
		GLTF::copyAttribute("NORMAL", normals, primitive, *gltf);
		GLTF::copyAttribute("TEXCOORD_0", uvs, primitive, *gltf);

		// Ensure all attributes have the same size
		AGX_ASSERT_X(!positions.empty(), "Failed to load positions");
		auto vertexCount = positions.size();
		if (colors.size() != vertexCount)
			colors.resize(vertexCount);

		if (normals.size() != vertexCount)
			normals.resize(vertexCount);

		if (uvs.size() != vertexCount)
			uvs.resize(vertexCount);
	}
}
