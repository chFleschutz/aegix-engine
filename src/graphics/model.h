#pragma once

#include "graphics/buffer.h"
#include "graphics/device.h"
#include "utils/math_utils.h"

#include <filesystem>
#include <memory>
#include <vector>

namespace Aegix::Graphics
{
	class StaticMesh
	{
	public:
		struct Vertex
		{
			Vector3 position{};
			Vector3 color{};
			Vector3 normal{};
			Vector2 uv{};

			static std::vector<VkVertexInputBindingDescription> bindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> attributeDescriptions();

			bool operator==(const Vertex& other) const 
			{
				return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
			}
		};

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
		~StaticMesh();

		StaticMesh(const StaticMesh&) = delete;
		StaticMesh& operator=(const StaticMesh&) = delete;

		static std::unique_ptr<StaticMesh> createModelFromFile(VulkanDevice& device, const std::filesystem::path& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		template<typename T>
		void createVertexAttributeBuffer(const std::vector<T>& attribute)
		{
			assert(attribute.size() >= 3 && "Vertex attribute must have at least 3 elements");

			size_t bufferSize = sizeof(T) * attribute.size();
			uint32_t instanceCount = static_cast<uint32_t>(attribute.size());

			Buffer stagingBuffer{ 
				m_device, 
				sizeof(T), 
				instanceCount,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT 
			};

			stagingBuffer.map();
			stagingBuffer.writeToBuffer(attribute.data(), bufferSize);
			stagingBuffer.unmap();

			auto attributeBuffer = std::make_unique<Buffer>(
				m_device,
				sizeof(T),
				instanceCount,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);
			
			m_device.copyBuffer(stagingBuffer.buffer(), attributeBuffer->buffer(), bufferSize);

			m_vkBuffers.emplace_back(attributeBuffer->buffer());
			m_bufferOffsets.emplace_back(0);
			m_attributeBuffers.emplace_back(std::move(attributeBuffer));
		}

		void createVertexBuffers(const std::vector<Vertex>& vertices);
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
