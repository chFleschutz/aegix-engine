#pragma once

#include "graphics/frame_graph/frame_graph_node.h"
#include "graphics/frame_graph/frame_graph_pass.h"
#include "graphics/frame_graph/frame_graph_resource_pool.h"
#include "graphics/descriptors.h"
#include "graphics/vulkan_tools.h"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Aegix::Graphics
{
	struct RendererData
	{
		VulkanDevice& device;
		DescriptorPool& pool;
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
					 std::is_invocable_v<Execute, const Data&, FrameGraphResourcePool&, const FrameInfo&> &&
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

		FrameGraph() = default;
		FrameGraph(const FrameGraph&) = delete;
		FrameGraph(FrameGraph&&) = delete;
		~FrameGraph() = default;

		FrameGraph& operator=(const FrameGraph&) = delete;
		FrameGraph& operator=(FrameGraph&&) = delete;

		[[nodiscard]]
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
				placeBarriers(frameInfo.commandBuffer, node);

				node.executePass(m_resourcePool, frameInfo);
			}
		}

		void swapChainResized()
		{
			// TODO
		}

		auto resourcePool() -> FrameGraphResourcePool& { return m_resourcePool; }

	private:
		void placeBarriers(VkCommandBuffer commandBuffer, FrameGraphNode& node)
		{
			std::vector<VkImageMemoryBarrier> readDepthBarriers;
			std::vector<VkImageMemoryBarrier> readColorBarriers;
			readColorBarriers.reserve(node.reads().size());

			for (auto& read : node.reads())
			{
				Texture& texture = m_resourcePool.texture(read).texture;

				VkImageLayout newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				
				if (texture.layout() == newLayout)
					continue;

				if (Tools::isDepthFormat(texture.format()))
				{
					readDepthBarriers.emplace_back(texture.imageMemoryBarrier(newLayout));
				}
				else
				{
					readColorBarriers.emplace_back(texture.imageMemoryBarrier(newLayout));
				}
			}

			Tools::vk::cmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				readDepthBarriers
			);

			Tools::vk::cmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				readColorBarriers
			);

			std::vector<VkImageMemoryBarrier> writeDepthBarriers;
			std::vector<VkImageMemoryBarrier> writeColorBarriers;
			writeColorBarriers.reserve(node.writes().size());

			for (auto& write : node.writes())
			{
				Texture& texture = m_resourcePool.texture(write).texture;

				if (Tools::isDepthFormat(texture.format()))
				{
					VkImageLayout newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					if (texture.layout() != newLayout)
						writeDepthBarriers.emplace_back(texture.imageMemoryBarrier(newLayout));
				}
				else
				{
					VkImageLayout newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					if (texture.layout() != newLayout)
						writeColorBarriers.emplace_back(texture.imageMemoryBarrier(newLayout));
				}
			}

			Tools::vk::cmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				writeDepthBarriers
			);

			Tools::vk::cmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				writeColorBarriers
			);
		}

		std::vector<FrameGraphNode> m_nodes;
		FrameGraphResourcePool m_resourcePool;
	};
}