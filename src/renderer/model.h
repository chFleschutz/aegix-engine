#pragma once

#include "utils/math_utils.h"
#include "renderer/buffer.h"
#include "renderer/device.h"

#include <filesystem>
#include <memory>
#include <vector>

namespace VEGraphics
{
	class Model
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

		struct Builder
		{
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::filesystem::path& filepath);
		};

		Model(VulkanDevice& device, const Model::Builder& builder);
		~Model();

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;

		static std::unique_ptr<Model> createModelFromFile(VulkanDevice& device, const std::filesystem::path& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		VulkanDevice& m_device;

		std::unique_ptr<Buffer> m_vertexBuffer;
		uint32_t m_vertexCount;

		bool m_hasIndexBuffer = false;
		std::unique_ptr<Buffer> m_indexBuffer;
		uint32_t m_indexCount;
	};

} // namespace vre
