#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/frame_info.h"
#include "graphics/texture.h"

#include <string>
#include <vector>

namespace Aegix::Graphics
{
	class FrameGraph
	{
	public:
		FrameGraph(VulkanDevice& device) : m_device{ device } {};
		FrameGraph(const FrameGraph&) = delete;
		~FrameGraph() = default;

		// TODO: Sort passes based on dependencies
		// TODO: Refactor this into a Builder class
		void buildDependencies()
		{
			for (auto& pass : m_passes)
			{
				pass->create();
			}
		}

		void execute(FrameInfo& frameInfo)
		{
			for (auto& pass : m_passes)
			{
				pass->execute(frameInfo);
			}
		}

		void registerCallback(const std::string& name, std::function<void()> callback)
		{
			m_callbacks[name] = callback;
		}

		std::shared_ptr<FrameGraphResource> createResource(const std::string& name, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage)
		{
			auto texture = std::make_shared<Texture>(m_device, width, height, format, usage);
			auto resource = std::make_shared<FrameGraphResource>(name, texture);
			m_resources[name] = resource;
			return resource;
		}

		template<typename T>
		std::shared_ptr<FrameGraphPass> createPass(std::string name)
		{
			static_assert(std::is_base_of<FrameGraphPass, T>::value, "T must derive from FrameGraphPass");

			auto pass = std::make_shared<T>(m_device);
			m_passes.push_back(pass);

			auto it = m_callbacks.find(name);
			assert(it != m_callbacks.end() && "Callback for FrameGraphPass was not found");
			
			pass->setExecuteCallback(it->second);
			return pass;
		}

	private:
		VulkanDevice& m_device;

		std::unordered_map<std::string, std::function<void()>> m_callbacks;
		std::unordered_map<std::string, std::shared_ptr<FrameGraphResource>> m_resources;
		std::vector<std::shared_ptr<FrameGraphPass>> m_passes;
	};
}