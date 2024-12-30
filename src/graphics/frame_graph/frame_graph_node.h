#pragma once

#include <string>

namespace Aegix::Graphics
{
	class FrameGraphNode
	{
	public:
		FrameGraphNode(std::string_view name, uint32_t id)
			: m_name{ name }, m_id{ id }
		{
		}

	private:
		std::string m_name;
		uint32_t m_id;
	};
}