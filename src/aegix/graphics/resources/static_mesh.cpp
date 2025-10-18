#include "pch.h"
#include "static_mesh.h"

#include "graphics/vulkan/volk_include.h"

namespace Aegix::Graphics
{
	auto StaticMesh::bindingDescription() -> VkVertexInputBindingDescription
	{
		static VkVertexInputBindingDescription bindingDescription{
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};
		return bindingDescription;
	}

	auto StaticMesh::attributeDescriptions() -> std::vector<VkVertexInputAttributeDescription>
	{
		static auto attributeDescriptions = std::vector{
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

	auto StaticMesh::meshletDescriptorSetLayout() -> DescriptorSetLayout&
	{
		static DescriptorSetLayout meshletDescriptorSetLayout = DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT) // Meshlet 
			.build();
		return meshletDescriptorSetLayout;
	}

	auto StaticMesh::attributeDescriptorSetLayout() -> DescriptorSetLayout&
	{
		static DescriptorSetLayout attributeDescriptorSetLayout = DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT) // Meshlet Indices 
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT) // Meshlet Primitives 
			.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_MESH_BIT_EXT) // Vertices
			.build();
		return attributeDescriptorSetLayout;
	}

	StaticMesh::StaticMesh(const CreateInfo& info) :
		m_vertexBuffer{ sizeof(Vertex) * info.vertices.size(), 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT },
		m_indexBuffer{ Buffer::createIndexBuffer(sizeof(uint32_t) * info.indices.size()) },
		m_meshletBuffer{ Buffer::createStorageBuffer(sizeof(Meshlet) * info.meshlets.size()) },
		m_meshletIndexBuffer{ Buffer::createStorageBuffer(sizeof(uint32_t) * info.vertexIndices.size()) },
		m_meshletPrimitiveBuffer{ Buffer::createStorageBuffer(sizeof(uint8_t) * info.primitiveIndices.size()) },
		m_meshletDescriptor{ meshletDescriptorSetLayout() },
		m_attributeDescriptor{ attributeDescriptorSetLayout() },
		m_vertexCount{ static_cast<uint32_t>(info.vertices.size()) },
		m_indexCount{ static_cast<uint32_t>(info.indices.size()) },
		m_meshletCount{ static_cast<uint32_t>(info.meshlets.size()) },
		m_meshletIndexCount{ static_cast<uint32_t>(info.vertexIndices.size()) },
		m_meshletPrimitiveCount{ static_cast<uint32_t>(info.primitiveIndices.size()) }
	{
		m_vertexBuffer.upload(info.vertices);
		m_indexBuffer.upload(info.indices);
		m_meshletBuffer.upload(info.meshlets);
		m_meshletIndexBuffer.upload(info.vertexIndices);
		m_meshletPrimitiveBuffer.upload(info.primitiveIndices);

		DescriptorWriter{ meshletDescriptorSetLayout() }
			.writeBuffer(0, m_meshletBuffer)
			.update(m_meshletDescriptor);

		DescriptorWriter{ attributeDescriptorSetLayout() }
			.writeBuffer(0, m_meshletIndexBuffer)
			.writeBuffer(1, m_meshletPrimitiveBuffer)
			.writeBuffer(2, m_vertexBuffer)
			.update(m_attributeDescriptor);
	}

	void StaticMesh::draw(VkCommandBuffer cmd) const
	{
		VkBuffer vertexBuffers[] = { m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(cmd, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, m_indexCount, 1, 0, 0, 0);
	}

	void StaticMesh::drawMeshlets(VkCommandBuffer cmd) const
	{
		vkCmdDrawMeshTasksEXT(cmd, m_meshletCount, 1, 1);
	}
}