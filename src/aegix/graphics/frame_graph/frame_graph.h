#pragma once

#include "graphics/frame_graph/frame_graph_resource_pool.h"
#include "graphics/frame_info.h"
#include "graphics/render_context.h"

namespace Aegix::Graphics
{
	class FGRenderPass;

	/// @brief Manages renderpasses and resources for rendering a frame
	class FrameGraph
	{
	public:
		FrameGraph() = default;
		FrameGraph(const FrameGraph&) = delete;
		FrameGraph(FrameGraph&&) = delete;
		~FrameGraph() = default;

		auto operator=(const FrameGraph&) -> FrameGraph = delete;
		auto operator=(FrameGraph&&) -> FrameGraph = delete;

		template<typename T, typename... Args>
			requires std::is_base_of_v<FGRenderPass, T> && std::constructible_from<T, FGResourcePool&, Args...>
		auto add(Args&&... args) -> T&
		{
			auto handle = m_pool.addNode(std::make_unique<T>(m_pool, std::forward<Args>(args)...));
			m_nodes.emplace_back(handle);
			return static_cast<T&>(*m_pool.node(handle).pass);
		}

		[[nodiscard]] auto nodes() -> std::vector<FGNodeHandle>& { return m_nodes; }
		[[nodiscard]] auto resourcePool() -> FGResourcePool& { return m_pool; }

		/// @brief Compiles the frame graph by sorting the nodes and creating resources
		void compile();

		/// @brief Executes the frame graph by executing each node in order
		void execute(const FrameInfo& frameInfo);

		/// @brief Resizes all swapchain relative resources textures
		void swapChainResized(uint32_t width, uint32_t height);

	private:
		using DependencyGraph = std::vector<std::vector<FGNodeHandle>>;

		auto buildDependencyGraph() -> DependencyGraph;
		auto topologicalSort(const DependencyGraph& adjacency) -> std::vector<FGNodeHandle>;
		void createResources();
		void generateBarriers();
		void generateBarrier(FGNode& node, FGResourceHandle srcHandle, FGResourceHandle dstHandle, 
			FGResourceHandle actualHandle);
		void placeBarriers(VkCommandBuffer cmd, const FGNode& node);

		std::vector<FGNodeHandle> m_nodes; // Sorted in execution order
		FGResourcePool m_pool;
	};
}