#pragma once

#include "graphics/frame_graph/frame_graph_node.h"
#include "graphics/frame_graph/frame_graph_pass.h"
#include "graphics/frame_graph/frame_graph_resource_pool.h"
#include "graphics/descriptors.h"
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
		struct NoData {};

		class Builder
		{
		public:
			Builder(FrameGraph& frameGraph, FrameGraphNode& node);
			FrameGraphResourceID declareRead(FrameGraphResourceID resource);
			FrameGraphResourceID declareWrite(FrameGraphResourceID resource);
			
		private:
			FrameGraph& m_frameGraph;
			FrameGraphNode& m_node;
		};

		FrameGraph() = default;
		FrameGraph(const FrameGraph&) = delete;
		FrameGraph(FrameGraph&&) = delete;
		~FrameGraph() = default;

		FrameGraph& operator=(const FrameGraph&) = delete;
		FrameGraph& operator=(FrameGraph&&) = delete;

		template <typename Data = NoData, typename Setup, typename Execute>
			requires std::is_invocable_v<Setup, Builder&, Data&> && 
					 std::is_invocable_v<Execute, const Data&, FrameGraphResourcePool&, const FrameInfo&> &&
					 (sizeof(Execute) < 1024)
		[[nodiscard]] auto addPass(const std::string& name, Setup&& setup, Execute&& execute) -> const Data&
		{
			auto pass = std::make_unique<FrameGraphPass<Data, Execute>>(std::forward<Execute>(execute));
			auto& data = pass->data;
			auto& node = m_nodes.emplace_back(name, static_cast<FrameGraphNodeID>(m_nodes.size()), std::move(pass));
			Builder builder{ *this, node };
			std::invoke(std::forward<Setup>(setup), builder, data);
			return data;
		}

		[[nodiscard]] auto addTexture(VulkanDevice& device, const std::string& name, const FrameGraphTexture::Desc& desc) -> FrameGraphResourceID;

		[[nodiscard]] auto resourcePool() -> FrameGraphResourcePool& { return m_resourcePool; }

		void compile();
		void execute(const FrameInfo& frameInfo);
		void swapChainResized();

	private:
		struct BarrierPlacement
		{
			VkImageLayout newLayout;
			VkPipelineStageFlagBits srcStage;
			VkPipelineStageFlagBits dstStage;
		};

		void placeBarriers(VkCommandBuffer commandBuffer, FrameGraphNode& node);
		void placeBarriers(VkCommandBuffer commandBuffer, const std::vector<FrameGraphResourceID>& resources,
			const BarrierPlacement& color, const BarrierPlacement& depth);

		std::vector<FrameGraphNode> m_nodes;
		FrameGraphResourcePool m_resourcePool;
	};
}