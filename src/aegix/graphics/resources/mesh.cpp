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
		vertexBuffer{ Buffer::createVertexBuffer(sizeof(Vertex), info.vertexCount) },
		indexBuffer{ Buffer::createIndexBuffer(sizeof(uint32_t), info.indexCount) },
		meshletBuffer{ Buffer::createStorageBuffer(sizeof(Meshlet), info.meshletCount) },
		positonBuffer{ Buffer::createStorageBuffer(sizeof(Vertex::position), info.vertexCount) },
		normalBuffer{ Buffer::createStorageBuffer(sizeof(Vertex::normal), info.vertexCount) },
		uvBuffer{ Buffer::createStorageBuffer(sizeof(Vertex::uv), info.vertexCount) },
		colorBuffer{ Buffer::createStorageBuffer(sizeof(Vertex::color), info.vertexCount) },
		meshletDescriptor{ meshletDescriptorSetLayout() },
		attributeDescriptor{ attributeDescriptorSetLayout() }
	{
	}

	void Mesh::Draw(VkCommandBuffer cmd) const
	{
		VkBuffer vertexBuffers[] = { vertexBuffer };
		vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, 0);
		vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, indexBuffer.instanceCount(), 1, 0, 0, 0);
	}

	void Mesh::DrawMeshlets(VkCommandBuffer cmd) const
	{
		// TODO: Bind descriptor set for meshlet buffer

		vkCmdDrawMeshTasksEXT(cmd, meshletBuffer.instanceCount(), 1, 1);
	}
}