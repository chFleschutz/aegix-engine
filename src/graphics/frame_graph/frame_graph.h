#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/frame_graph_node.h"
#include "graphics/frame_graph/frame_graph_pass.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/frame_graph/frame_graph_resource_pool.h"
#include "graphics/vulkan_tools.h"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Aegix::Graphics
{
	struct RendererData
	{
		VulkanDevice& device;
		DescriptorPool& pool;
	};

	class FrameGraph
	{
	public:
		FrameGraph() = default;
		FrameGraph(const FrameGraph&) = delete;
		FrameGraph(FrameGraph&&) = delete;
		~FrameGraph() = default;

		FrameGraph& operator=(const FrameGraph&) = delete;
		FrameGraph& operator=(FrameGraph&&) = delete;

		template<typename T, typename... Args>
			requires std::is_base_of_v<FrameGraphRenderPass, T> && std::is_constructible_v<T, Args...>
		void add(Args&&... args)
		{
			auto handle = m_resourcePool.addNode<T>(std::forward<Args>(args)...);
			m_nodes.emplace_back(handle);
		}

		[[nodiscard]] auto addTexture(VulkanDevice& device, const std::string& name, uint32_t width, uint32_t height, VkFormat format, 
			VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) -> FrameGraphResourceHandle;
		[[nodiscard]] auto addTexture(VulkanDevice& device, const std::string& name, VkFormat format, 
			VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) -> FrameGraphResourceHandle;

		[[nodiscard]] auto resourcePool() -> FrameGraphResourcePool& { return m_resourcePool; }

		void compile();
		void execute(const FrameInfo& frameInfo);
		void swapChainResized(VulkanDevice& device, uint32_t width, uint32_t height);

	private:
		struct BarrierPlacement
		{
			VkImageLayout newLayout;
			VkPipelineStageFlagBits srcStage;
			VkPipelineStageFlagBits dstStage;
		};

		void placeBarriers(VkCommandBuffer commandBuffer, FrameGraphNode& node);
		void placeBarriers(VkCommandBuffer commandBuffer, const std::vector<FrameGraphResourceHandle>& resources,
			const BarrierPlacement& color, const BarrierPlacement& depth);

		std::vector<FrameGraphNodeHandle> m_nodes;
		FrameGraphResourcePool m_resourcePool;
	};
}