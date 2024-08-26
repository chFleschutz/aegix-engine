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
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadOBJ(const std::filesystem::path& filepath);
			void loadGLTF(const std::filesystem::path& filepath);
		};

		StaticMesh(VulkanDevice& device, const StaticMesh::MeshInfo& builder);
		~StaticMesh();

		StaticMesh(const StaticMesh&) = delete;
		StaticMesh& operator=(const StaticMesh&) = delete;

		static std::unique_ptr<StaticMesh> createModelFromFile(VulkanDevice& device, const std::filesystem::path& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		VulkanDevice& m_device;

		std::unique_ptr<Buffer> m_vertexBuffer;
		uint32_t m_vertexCount;

		std::unique_ptr<Buffer> m_indexBuffer;
		uint32_t m_indexCount;
	};
}
