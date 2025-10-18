#pragma once

#include "graphics/resources/buffer.h"
#include "graphics/descriptors.h"

namespace Aegix::Graphics
{
	class StaticMesh
	{
	public:
		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 uv;
			glm::vec3 color;
		};

		struct Meshlet
		{
			glm::vec3 center;
			float radius;
			int8_t coneAxis[3];
			int8_t coneCutoff;
			uint32_t vertexOffset;
			uint32_t primitiveOffset;
			uint8_t vertexCount;
			uint8_t primitiveCount;
		};

		struct CreateInfo
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::vector<Meshlet> meshlets;
			std::vector<uint32_t> vertexIndices;
			std::vector<uint8_t> primitiveIndices;
		};

		static auto bindingDescription() -> VkVertexInputBindingDescription;
		static auto attributeDescriptions() -> std::vector<VkVertexInputAttributeDescription>;
		static auto meshletDescriptorSetLayout() -> DescriptorSetLayout&;
		static auto attributeDescriptorSetLayout() -> DescriptorSetLayout&;

		StaticMesh(const CreateInfo& info);
		StaticMesh(const StaticMesh&) = delete;
		StaticMesh(StaticMesh&&) = default;
		~StaticMesh() = default;

		auto operator=(const StaticMesh&) -> StaticMesh& = delete;
		auto operator=(StaticMesh&&) -> StaticMesh& = default;

		[[nodiscard]] auto vertexCount() const -> uint32_t { return m_vertexCount; }
		[[nodiscard]] auto indexCount() const -> uint32_t { return m_indexCount; }
		[[nodiscard]] auto meshletCount() const -> uint32_t { return m_meshletCount; }
		[[nodiscard]] auto meshletDescriptorSet() const -> const DescriptorSet& { return m_meshletDescriptor; }
		[[nodiscard]] auto attributeDescriptorSet() const -> const DescriptorSet& { return m_attributeDescriptor; }

		void draw(VkCommandBuffer cmd) const;
		void drawMeshlets(VkCommandBuffer cmd) const;

	private:
		// Vertex and index buffers for traditional rendering
		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;

		// Storage buffers for mesh shaders
		Buffer m_meshletBuffer;
		Buffer m_meshletIndexBuffer;
		Buffer m_meshletPrimitiveBuffer;
		DescriptorSet m_meshletDescriptor;
		DescriptorSet m_attributeDescriptor;

		uint32_t m_vertexCount{};
		uint32_t m_indexCount{};
		uint32_t m_meshletCount{};
		uint32_t m_meshletIndexCount{};
		uint32_t m_meshletPrimitiveCount{};
	};
}