#pragma once

#include "graphics/frame_graph/frame_graph_node.h"
#include "graphics/frame_graph/frame_graph_pass.h"
#include "graphics/frame_graph/frame_graph_resource_pool.h"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Aegix::Graphics
{

	class FrameGraph
	{
	public:
		struct NoData {};

		class Builder
		{
		public:
			Builder(FrameGraph& frameGraph, FrameGraphNode& node)
				: m_frameGraph{ frameGraph }, m_node{ node }
			{
			}

			FrameGraphResourceID declareRead(FrameGraphResourceID resource) 
			{ 
				m_node.addRead(resource); 
				return resource;
			}

			FrameGraphResourceID declareWrite(FrameGraphResourceID resource) 
			{ 
				m_node.addWrite(resource);
				return resource;
			}

		private:
			FrameGraph& m_frameGraph;
			FrameGraphNode& m_node;
		};

		template <typename Data = NoData, typename Setup, typename Execute>
			requires std::is_invocable_v<Setup, Builder&, Data&> && 
					 std::is_invocable_v<Execute, const Data&, const FrameGraphResourcePool&, const FrameInfo&> &&
					 (sizeof(Execute) < 1024)
		[[nodiscard]]
		const Data& addPass(const std::string& name, Setup&& setup, Execute&& execute)
		{
			auto pass = std::make_unique<FrameGraphPass<Data, Execute>>(std::forward<Execute>(execute));
			auto& data = pass->data;

			const auto id = static_cast<FrameGraphNodeID>(m_nodes.size());
			auto& node = m_nodes.emplace_back(name, id, std::move(pass));

			Builder builder{ *this, node };
			std::invoke(std::forward<Setup>(setup), builder, data);
			return data;
		}

		auto addTexture(VulkanDevice& device, const std::string& name, const FrameGraphTexture::Desc& desc) -> FrameGraphResourceID
		{
			return m_resourcePool.addTexture(device, name, desc);
		}

		void compile()
		{
			// TODO
		}

		void execute(const FrameInfo& frameInfo)
		{
			for (auto& node : m_nodes)
			{
				node.executePass(m_resourcePool, frameInfo);
			}
		}

		void swapChainResized()
		{
			// TODO
		}

	private:
		std::vector<FrameGraphNode> m_nodes;
		FrameGraphResourcePool m_resourcePool;
	};
}