#pragma once

#include "graphics/bindless/bindless_buffer.h"
#include "graphics/descriptors.h"
#include "graphics/resources/buffer.h"

#include <glm/glm.hpp>

namespace Aegis::Graphics
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

		struct BoundingSphere
		{
			glm::vec3 center;
			float radius;
		};

		struct Meshlet
		{
			BoundingSphere bounds;
			int8_t coneAxis[3];
			int8_t coneCutoff;
			uint32_t vertexOffset;
			uint32_t primitiveOffset;
			uint8_t vertexCount;
			uint8_t primitiveCount;
		};

		struct MeshData
		{
			DescriptorHandle vertexBuffer;
			DescriptorHandle indexBuffer;
			DescriptorHandle meshletBuffer;
			DescriptorHandle meshletVertexBuffer;
			DescriptorHandle meshletPrimitiveBuffer;
			uint32_t vertexCount;
			uint32_t indexCount;
			uint32_t meshletCount;
			BoundingSphere bounds;
		};

		struct CreateInfo
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			std::vector<Meshlet> meshlets;
			std::vector<uint32_t> vertexIndices;
			std::vector<uint8_t> primitiveIndices;
			BoundingSphere bounds;
		};

		static auto bindingDescription() -> VkVertexInputBindingDescription;
		static auto attributeDescriptions() -> std::vector<VkVertexInputAttributeDescription>;

		StaticMesh(const CreateInfo& info);
		StaticMesh(const StaticMesh&) = delete;
		StaticMesh(StaticMesh&&) = default;
		~StaticMesh() = default;

		auto operator=(const StaticMesh&) -> StaticMesh& = delete;
		auto operator=(StaticMesh&&) -> StaticMesh& = default;

		[[nodiscard]] auto vertexCount() const -> uint32_t { return m_vertexCount; }
		[[nodiscard]] auto indexCount() const -> uint32_t { return m_indexCount; }
		[[nodiscard]] auto meshletCount() const -> uint32_t { return m_meshletCount; }
		[[nodiscard]] auto meshDataBuffer() const -> const BindlessBuffer& { return m_meshDataBuffer; }

		void draw(VkCommandBuffer cmd) const;
		void drawMeshlets(VkCommandBuffer cmd) const;

	private:
		BindlessBuffer m_meshDataBuffer;
		BindlessBuffer m_vertexBuffer;
		BindlessBuffer m_indexBuffer;
		BindlessBuffer m_meshletBuffer;
		BindlessBuffer m_meshletVertexBuffer;
		BindlessBuffer m_meshletPrimitiveBuffer;

		uint32_t m_vertexCount;
		uint32_t m_indexCount;
		uint32_t m_meshletCount;
		uint32_t m_meshletIndexCount;
		uint32_t m_meshletPrimitiveCount;
	};
}