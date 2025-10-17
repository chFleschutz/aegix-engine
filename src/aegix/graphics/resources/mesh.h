#pragma once

#include "graphics/resources/buffer.h"
#include "graphics/descriptors.h"

namespace Aegix::Graphics
{
	class Mesh
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
			uint32_t triangleOffset;
			uint32_t triangleCount;
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
			std::vector<glm::vec3> positions;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec2> uvs;
			std::vector<glm::vec3> colors;
			uint32_t vertexCount;
			uint32_t indexCount;
			uint32_t meshletCount;
		};

		static auto bindingDescription() -> VkVertexInputBindingDescription;
		static auto attributeDescriptions() -> std::array<VkVertexInputAttributeDescription, 4>;
		static auto meshletDescriptorSetLayout() -> DescriptorSetLayout&;
		static auto attributeDescriptorSetLayout() -> DescriptorSetLayout&;

		Mesh(const CreateInfo& info);
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) = default;
		~Mesh() = default;

		auto operator=(const Mesh&) -> Mesh& = delete;
		auto operator=(Mesh&&) -> Mesh& = default;

		void Draw(VkCommandBuffer cmd) const;
		void DrawMeshlets(VkCommandBuffer cmd) const;

	private:
		// Vertex and index buffers for traditional rendering
		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;

		// Storage buffers for mesh shaders
		Buffer m_meshletBuffer;
		Buffer m_meshletIndexBuffer;
		Buffer m_positonBuffer;
		Buffer m_normalBuffer;
		Buffer m_uvBuffer;
		Buffer m_colorBuffer;
		DescriptorSet m_meshletDescriptor;
		DescriptorSet m_attributeDescriptor;
	};
}