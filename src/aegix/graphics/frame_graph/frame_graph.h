#pragma once

#include "graphics/frame_graph/frame_graph_resource_pool.h"
#include "graphics/frame_info.h"

namespace Aegix::Graphics
{
	/// @brief Manages renderpasses and resources for rendering a frame
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
		auto add(Args&&... args) -> FrameGraphNodeHandle
		{
			auto handle = m_resourcePool.addNode(std::make_unique<T>(std::forward<Args>(args)...));
			m_nodeHandles.emplace_back(handle);
			return handle;
		}

		[[nodiscard]] auto resourcePool() -> FrameGraphResourcePool& { return m_resourcePool; }
		[[nodiscard]] auto nodes() -> std::vector<FrameGraphNodeHandle>& { return m_nodeHandles; }

		/// @brief Compiles the frame graph by sorting the nodes and creating resources
		void compile();

		/// @brief Executes the frame graph by executing each node in order
		void execute(const FrameInfo& frameInfo);

		/// @brief Resizes all swapchain relative resources textures
		void swapChainResized(uint32_t width, uint32_t height);

	private:
		void computeEdges();
		void sortNodes();
		void placeBarriers(VkCommandBuffer commandBuffer, FrameGraphNode& node);

		std::vector<FrameGraphNodeHandle> m_nodeHandles;
		FrameGraphResourcePool m_resourcePool;
	};
}