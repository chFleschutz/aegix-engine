#pragma once

#include "graphics/device.h"

#include <functional>
#include <string>
#include <vector>

namespace Aegix::Graphics
{
	struct RenderPass
	{
		std::string name;
		std::function<void(VkCommandBuffer)> execute;
	};

	class FrameGraph
	{
	public:
		FrameGraph(VulkanDevice& device) : m_device{ device } {};
		FrameGraph(const FrameGraph&) = delete;
		~FrameGraph() = default;

		RenderPass& addPass(const std::string& name, 
			std::function<void(RenderPass&)> setup, 
			std::function<void(VkCommandBuffer)> execute)
		{
			auto& pass = m_renderPasses.emplace_back(name, execute);
			setup(pass);
			return pass;
		}

		void compile()
		{
			// TODO
		}

		void execute(VkCommandBuffer cmdBuffer)
		{
			for (auto& pass : m_renderPasses)
			{
				pass.execute(cmdBuffer);
			}
		}

	private:
		VulkanDevice& m_device;

		std::vector<RenderPass> m_renderPasses;
	};
}