#pragma once

#include "graphics/device.h"
#include "graphics/resources/buffer.h"

#include "glm/glm.hpp"

namespace Aegix::Graphics
{
	class StaticMesh
	{
	public:
		struct MeshInfo
		{
			std::vector<glm::vec3> positions{};
			std::vector<glm::vec3> colors{};
			std::vector<glm::vec3> normals{};
			std::vector<glm::vec2> uvs{};
			std::vector<uint32_t> indices{};

			void loadOBJ(const std::filesystem::path& filepath);
			void loadGLTF(const std::filesystem::path& filepath);
		};

		StaticMesh(VulkanDevice& device, const StaticMesh::MeshInfo& info);
		~StaticMesh() = default;

		StaticMesh(const StaticMesh&) = delete;
		StaticMesh& operator=(const StaticMesh&) = delete;

		static std::vector<VkVertexInputBindingDescription> defaultBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> defaultAttributeDescriptions();
		static std::shared_ptr<StaticMesh> create(const std::filesystem::path& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		template<typename T>
		void createVertexAttributeBuffer(const std::vector<T>& attribute)
		{
			assert(attribute.size() >= 3 && "Vertex attribute must have at least 3 elements");

			size_t bufferSize = sizeof(T) * attribute.size();
			auto attributeBuffer = std::make_unique<Buffer>(m_device, bufferSize, 1,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
			attributeBuffer->upload(attribute.data(), bufferSize);

			m_vkBuffers.emplace_back(attributeBuffer->buffer());
			m_bufferOffsets.emplace_back(0);
			m_attributeBuffers.emplace_back(std::move(attributeBuffer));
		}

		void createIndexBuffers(const std::vector<uint32_t>& indices);

		VulkanDevice& m_device;
		
		std::vector<std::unique_ptr<Buffer>> m_attributeBuffers;
		uint32_t m_vertexCount;

		std::unique_ptr<Buffer> m_indexBuffer;
		uint32_t m_indexCount;

		std::vector<VkBuffer> m_vkBuffers;
		std::vector<VkDeviceSize> m_bufferOffsets;
	};
}
