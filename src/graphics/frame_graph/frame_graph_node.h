#pragma once

#include "graphics/frame_graph/frame_graph_pass.h"
#include "graphics/frame_graph/frame_graph_resource.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace Aegix::Graphics
{
	using FrameGraphNodeID = uint32_t;

	class FrameGraphNode
	{
	public:
		friend class FrameGraph;

		FrameGraphNode(std::string_view name, FrameGraphNodeID id, std::unique_ptr<FrameGraphPassBase> pass)
			: m_name{ name }, m_id{ id }, m_pass{ std::move(pass) }
		{
		}

		void executePass(FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
		{
			std::invoke(*m_pass, resources, frameInfo);
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
		std::string m_name;
		FrameGraphNodeID m_id;
		std::unique_ptr<FrameGraphPassBase> m_pass;
		std::vector<FrameGraphResourceID> m_reads;
		std::vector<FrameGraphResourceID> m_writes;
	};
}