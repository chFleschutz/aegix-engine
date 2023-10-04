#pragma once

#include "buffer.h"
#include "device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace vre
{
	class VreModel
	{
	public:
		struct Vertex
		{
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};

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

			void loadModel(const std::string& filepath);
		};

		VreModel(VreDevice& device, const VreModel::Builder& builder);
		~VreModel();

		VreModel(const VreModel&) = delete;
		VreModel& operator=(const VreModel&) = delete;

		static std::unique_ptr<VreModel> createModelFromFile(VreDevice& device, const std::string& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		VreDevice& mVreDevice;

		std::unique_ptr<VreBuffer> mVertexBuffer;
		uint32_t mVertexCount;

		bool mHasIndexBuffer = false;
		std::unique_ptr<VreBuffer> mIndexBuffer;
		uint32_t mIndexCount;
	};

} // namespace vre
