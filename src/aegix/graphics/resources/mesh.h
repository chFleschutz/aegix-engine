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
			uint32_t indexOffset;
			uint32_t indexCount;
			glm::vec3 center;
			float radius;
			glm::vec3 coneAxis;
			float coneCutoff;
		};

		struct CreateInfo
		{
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
		Buffer vertexBuffer;
		Buffer indexBuffer;

		// Storage buffers for mesh shading
		Buffer meshletBuffer;
		Buffer positonBuffer;
		Buffer normalBuffer;
		Buffer uvBuffer;
		Buffer colorBuffer;
		DescriptorSet meshletDescriptor;
		DescriptorSet attributeDescriptor;
	};
}