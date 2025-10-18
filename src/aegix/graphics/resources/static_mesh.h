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
			uint32_t vertexOffset;
			uint32_t vertexCount;
			uint32_t primitiveOffset;
			uint32_t primitiveCount;
			glm::vec3 center;
			float radius;
			glm::vec3 coneAxis;
			float coneCutoff;
		};

		struct CreateInfo
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			std::vector<Meshlet> meshlets;
			std::vector<uint32_t> meshletIndices;
			std::vector<uint8_t> meshletPrimitives;
			std::vector<glm::vec3> positions;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec2> uvs;
			std::vector<glm::vec3> colors;

			uint32_t vertexCount;
			uint32_t indexCount;
			uint32_t meshletCount;
			uint32_t meshletIndexCount;
			uint32_t meshletPrimitiveCount;
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
		Buffer m_positonBuffer;
		Buffer m_normalBuffer;
		Buffer m_uvBuffer;
		Buffer m_colorBuffer;
		DescriptorSet m_meshletDescriptor;
		DescriptorSet m_attributeDescriptor;

		uint32_t m_vertexCount{};
		uint32_t m_indexCount{};
		uint32_t m_meshletCount{};
		uint32_t m_meshletIndexCount{};
		uint32_t m_meshletPrimitiveCount{};
	};
}