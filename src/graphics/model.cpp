#include "model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <cassert>


namespace Aegix::Graphics
{
	StaticMesh::StaticMesh(VulkanDevice& device, const StaticMesh::MeshInfo& info) : m_device{ device }
	{
		// Create vertex buffers (Order: Position, Color, Normal, UV)
		m_vertexCount = static_cast<uint32_t>(info.positions.size());
		assert(info.colors.size() == m_vertexCount && info.normals.size() == m_vertexCount && info.uvs.size() == m_vertexCount
			&& "Vertex attribute count has to match");

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

	std::unique_ptr<StaticMesh> StaticMesh::createModelFromFile(VulkanDevice& device, const std::filesystem::path& filepath)
	{
		MeshInfo builder{};
		builder.loadOBJ(filepath);

		return std::make_unique<StaticMesh>(device, builder);
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
		uint32_t indexSize = sizeof(indices[0]);

		Buffer stagingBuffer{
			m_device,
			indexSize,
			m_indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		m_indexBuffer = std::make_unique<Buffer>(
			m_device,
			indexSize,
			m_indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		m_device.copyBuffer(stagingBuffer.buffer(), m_indexBuffer->buffer(), bufferSize);
	}

	void StaticMesh::MeshInfo::loadOBJ(const std::filesystem::path& filepath)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.string().c_str()))
			throw std::runtime_error(warn + err);

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

	}
}
