#pragma once

#include "graphics/frame_graph/frame_graph_node.h"
#include "graphics/frame_graph/frame_graph_pass.h"

#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace Aegix::Graphics
{
	class PassNode : public FrameGraphNode
	{
	public:
		PassNode(std::string_view name, uint32_t id, std::unique_ptr<FrameGraphPassBase> pass)
			: FrameGraphNode{ name, id }, m_pass{ std::move(pass) }
		{
		}

		void execute() const
		{
			std::invoke(*m_pass);
		}

	private:
		std::unique_ptr<FrameGraphPassBase> m_pass;
	};

	class FrameGraph
	{
	public:
		struct NoData {};

		class Builder
		{
		public:
			Builder(FrameGraph& frameGraph, PassNode& node)
				: m_frameGraph{ frameGraph }, m_node{ node }
			{
			}

		private:
			FrameGraph& m_frameGraph;
			PassNode& m_node;
		};

		template <typename Data = NoData, typename Setup, typename Execute>
		const Data& addPass(const std::string& name, Setup&& setup, Execute&& execute)
		{
			static_assert(std::is_invocable_v<Setup, Builder&, Data&>, "Invalid setup lambda");
			static_assert(std::is_invocable_v<Execute, const Data&>, "Invalid execute lambda");
			static_assert(sizeof(Execute) < 1024, "Execute lambda captures too much (max 1024 bytes)");

			auto pass = std::make_unique<FrameGraphPass<Data, Execute>>(std::forward<Execute>(execute));
			auto& data = pass->data;

			const auto id = static_cast<uint32_t>(m_passes.size());
			auto& node = m_passes.emplace_back(name, id, std::move(pass));

			Builder builder{ *this, node };
			std::invoke(setup, builder, data);
			return data;
		}

		void compile()
		{
			// TODO
		}

		void execute()
		{
			for (auto& pass : m_passes)
			{
				pass.execute();
			}
		}

	private:
		std::vector<PassNode> m_passes;
	};
}