#include "pch.h"
#include "mesh.h"

#include "graphics/vulkan/volk_include.h"

namespace Aegix::Graphics
{
	auto Mesh::bindingDescription() -> VkVertexInputBindingDescription
	{
		static VkVertexInputBindingDescription bindingDescription{
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};
		return bindingDescription;
	}

	auto Mesh::attributeDescriptions() -> std::array<VkVertexInputAttributeDescription, 4>
	{
		static auto attributeDescriptions = std::array{
			VkVertexInputAttributeDescription{
				.location = 0,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, position)
			},
			VkVertexInputAttributeDescription{
				.location = 1,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, normal)
			},
			VkVertexInputAttributeDescription{
				.location = 2,
				.binding = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = offsetof(Vertex, uv)
			},
			VkVertexInputAttributeDescription{
				.location = 3,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, color)
			}
		};
		return attributeDescriptions;
	}

	auto Mesh::meshletDescriptorSetLayout() -> DescriptorSetLayout&
	{
		static DescriptorSetLayout meshletDescriptorSetLayout = DescriptorSetLayout::Builder()
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT) // Meshlet 
			.build();
		return meshletDescriptorSetLayout;
	}

	auto Mesh::attributeDescriptorSetLayout() -> DescriptorSetLayout&
	{
		static DescriptorSetLayout attributeDescriptorSetLayout = DescriptorSetLayout::Builder()
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT) // Position 
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT) // Normal 
			.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT) // UV 
			.addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT) // Color 
			.build();
		return attributeDescriptorSetLayout;
	}

	Mesh::Mesh(const CreateInfo& info) :
		m_vertexBuffer{ Buffer::createVertexBuffer(sizeof(Vertex) * info.vertexCount) },
		m_indexBuffer{ Buffer::createIndexBuffer(sizeof(uint32_t) * info.indexCount) },
		m_meshletBuffer{ Buffer::createStorageBuffer(sizeof(Meshlet) * info.meshletCount) },
		m_meshletIndexBuffer{ Buffer::createStorageBuffer(sizeof(uint32_t) * info.indexCount) }, // TODO: check size
		m_positonBuffer{ Buffer::createStorageBuffer(sizeof(Vertex::position) * info.vertexCount) },
		m_normalBuffer{ Buffer::createStorageBuffer(sizeof(Vertex::normal) * info.vertexCount) },
		m_uvBuffer{ Buffer::createStorageBuffer(sizeof(Vertex::uv) * info.vertexCount) },
		m_colorBuffer{ Buffer::createStorageBuffer(sizeof(Vertex::color) * info.vertexCount) },
		m_meshletDescriptor{ meshletDescriptorSetLayout() },
		m_attributeDescriptor{ attributeDescriptorSetLayout() }
	{
		AGX_ASSERT_X(info.vertices.size() == info.vertexCount, "Vertex count does not match size of vertex array");
		AGX_ASSERT_X(info.indices.size() == info.indexCount, "Index count does not match size of index array");
		AGX_ASSERT_X(info.meshlets.size() == info.meshletCount, "Meshlet count does not match size of meshlet array");
		AGX_ASSERT_X(info.positions.size() == info.vertexCount, "Vertex count does not match size of position array");
		AGX_ASSERT_X(info.normals.size() == info.vertexCount, "Vertex count does not match size of normal array");
		AGX_ASSERT_X(info.uvs.size() == info.vertexCount, "Vertex count does not match size of UV array");
		AGX_ASSERT_X(info.colors.size() == info.vertexCount, "Vertex count does not match size of color array");

		m_vertexBuffer.upload(info.vertices.data(), sizeof(Vertex) * info.vertexCount);
		m_indexBuffer.upload(info.indices.data(), sizeof(uint32_t) * info.indexCount);
		m_meshletBuffer.upload(info.meshlets.data(), sizeof(Meshlet) * info.meshletCount);
		m_meshletIndexBuffer.upload(info.indices.data(), sizeof(uint32_t) * info.indexCount);
		m_positonBuffer.upload(info.positions.data(), sizeof(Vertex::position) * info.vertexCount);
		m_normalBuffer.upload(info.normals.data(), sizeof(Vertex::normal) * info.vertexCount);
		m_uvBuffer.upload(info.uvs.data(), sizeof(Vertex::uv) * info.vertexCount);
		m_colorBuffer.upload(info.colors.data(), sizeof(Vertex::color) * info.vertexCount);

		DescriptorWriter{ meshletDescriptorSetLayout() }
		 	.writeBuffer(0, m_meshletBuffer)
			.update(m_meshletDescriptor);

		DescriptorWriter{ attributeDescriptorSetLayout() }
			.writeBuffer(0, m_meshletIndexBuffer)
			.writeBuffer(1, m_positonBuffer)
			.writeBuffer(2, m_normalBuffer)
			.writeBuffer(3, m_uvBuffer)
			.writeBuffer(4, m_colorBuffer)
			.update(m_attributeDescriptor);
	}

	void Mesh::Draw(VkCommandBuffer cmd) const
	{
		VkBuffer vertexBuffers[] = { m_vertexBuffer };
		vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, 0);
		vkCmdBindIndexBuffer(cmd, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, m_indexBuffer.instanceCount(), 1, 0, 0, 0);
	}

	void Mesh::DrawMeshlets(VkCommandBuffer cmd) const
	{
		// TODO: Bind descriptor set for meshlet buffer (pipeline layout needed)

		vkCmdDrawMeshTasksEXT(cmd, m_meshletBuffer.instanceCount(), 1, 1);
	}
}