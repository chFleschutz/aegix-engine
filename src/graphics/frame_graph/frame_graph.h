#pragma once

#include "graphics/frame_graph/frame_graph_node.h"
#include "graphics/frame_graph/frame_graph_pass.h"
#include "graphics/frame_graph/frame_graph_resource.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <string>
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

	class PassNode : public FrameGraphNode
	{
	public:
		friend class FrameGraph;

		PassNode(std::string_view name, uint32_t id, std::unique_ptr<FrameGraphPassBase> pass)
			: FrameGraphNode{ name, id }, m_pass{ std::move(pass) }
		{
		}

		void execute() const
		{
			std::invoke(*m_pass);
		}

		void addRead(FrameGraphResourceID resource)
		{
			assert(std::find(m_reads.begin(), m_reads.end(), resource) == m_reads.end() && 
				"Resource already declared as read");
			m_reads.emplace_back(resource);
		}

		void addWrite(FrameGraphResourceID resource)
		{
			assert(std::find(m_writes.begin(), m_writes.end(), resource) == m_writes.end() &&
				"Resource already declared as write");
			m_writes.emplace_back(resource);
		}

	private:
		std::unique_ptr<FrameGraphPassBase> m_pass;
		std::vector<FrameGraphResourceID> m_reads;
		std::vector<FrameGraphResourceID> m_writes;
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

			template <typename T>
				requires HasDesc<T>
			[[nodiscard]] 
			auto create(const std::string_view name, const typename T::Desc& desc) -> FrameGraphResourceID
			{
				return m_frameGraph.createResource<T>(name, desc);
			}

			void declareRead(FrameGraphResourceID resource) 
			{
				m_node.addRead(resource);
			}

			void declareWrite(FrameGraphResourceID resource)
			{
				m_node.addWrite(resource);
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
		template <typename T>
		auto createResource(const std::string_view name, const typename T::Desc& desc) -> FrameGraphResourceID
		{
			const auto id = static_cast<FrameGraphResourceID>(m_resources.size());
			m_resources.emplace_back(id, desc, T{});
			return id;
		}

		std::vector<PassNode> m_passes;
		std::vector<FrameGraphResource> m_resources;
	};
}