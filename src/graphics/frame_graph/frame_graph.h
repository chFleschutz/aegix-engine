#pragma once

#include "graphics/frame_graph/frame_graph_node.h"
#include "graphics/frame_graph/frame_graph_pass.h"
#include "graphics/frame_graph/frame_graph_resource.h"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Aegix::Graphics
{
	template <typename T>
	concept HasDesc = requires(T) { typename T::Desc; };


	class FrameGraphTexture
	{
	public:
		struct Desc
		{
			uint32_t width;
			uint32_t height;
		};
	};


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

			template <typename T>
				requires HasDesc<T>
			[[nodiscard]]
			auto create(const std::string_view name, const typename T::Desc& desc) -> FrameGraphResourceID
			{
				return m_frameGraph.addResource<T>(name, desc);
			}

			void declareRead(FrameGraphResourceID resource) { m_node.addRead(resource); }
			void declareWrite(FrameGraphResourceID resource) { m_node.addWrite(resource); }

		private:
			FrameGraph& m_frameGraph;
			FrameGraphNode& m_node;
		};

		template <typename Data = NoData, typename Setup, typename Execute>
			requires std::is_invocable_v<Setup, Builder&, Data&> && 
					 std::is_invocable_v<Execute, const Data&> &&
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

		template <typename T>
		auto addResource(const std::string_view name, const typename T::Desc& desc) -> FrameGraphResourceID
		{
			const auto id = static_cast<FrameGraphResourceID>(m_resources.size());
			m_resources.emplace_back(id, desc, T{});
			return id;
		}

		void compile()
		{
			// TODO
		}

		void execute()
		{
			for (auto& node : m_nodes)
			{
				node.executePass();
			}
		}

	private:
		std::vector<FrameGraphNode> m_nodes;
		std::vector<FrameGraphResource> m_resources;
	};
}