#include "pch.h"
#include "frame_graph.h"

#include "core/profiler.h"
#include "graphics/vulkan/vulkan_tools.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"

// Hash function for ResourceHandle to be used in unordered_map
namespace std
{
	template<>
	struct hash<Aegix::Graphics::FGResourceHandle>
	{
		auto operator()(const Aegix::Graphics::FGResourceHandle& handle) const noexcept -> size_t
		{
			return std::hash<uint32_t>()(handle.handle);
		}
	};
}

namespace Aegix::Graphics
{
	void FrameGraph::compile()
	{
		m_pool.resolveReferences();

		{
			auto graph = buildDependencyGraph();
			m_nodes = topologicalSort(graph);
		}

		// TODO: Compute resource lifetimes for aliasing

		m_pool.createResources();

		generateBarriers();
	}

	void FrameGraph::execute(const FrameInfo& frameInfo)
	{
		AGX_PROFILE_FUNCTION();

		for (auto nodeHandle : m_nodes)
		{
			auto& node = m_pool.node(nodeHandle);

			Tools::vk::cmdBeginDebugUtilsLabel(frameInfo.cmd, node.info.name.c_str());
			{
				Tools::vk::cmdPipelineBarrier(frameInfo.cmd, node.srcStage, node.dstStage,
					node.bufferBarriers, node.imageBarriers);

				node.pass->execute(m_pool, frameInfo);
			}
			Tools::vk::cmdEndDebugUtilsLabel(frameInfo.cmd);
		}
	}

	void FrameGraph::swapChainResized(uint32_t width, uint32_t height)
	{
		m_pool.resizeImages(width, height);

		for (const auto& nodeHandle : m_nodes)
		{
			auto& node = m_pool.node(nodeHandle);
			node.pass->createResources(m_pool);
		}
	}

	auto FrameGraph::buildDependencyGraph() -> DependencyGraph
	{
		// Register producers
		std::unordered_map<FGResourceHandle, FGNodeHandle> producers;
		for (const auto& FGNodeHandle : m_nodes)
		{
			auto& node = m_pool.node(FGNodeHandle);
			for (auto& write : node.info.writes)
			{
				const auto& resource = m_pool.resource(write);
				if (!std::holds_alternative<FGReferenceInfo>(resource.info))
				{
					producers[write] = FGNodeHandle;
				}
			}
		}

		// Build adjacency list
		std::vector<std::vector<FGNodeHandle>> adjacency(m_nodes.size());

		// Link write -> write dependencies
		for (const auto& FGNodeHandle : m_nodes)
		{
			auto& node = m_pool.node(FGNodeHandle);

			for (auto& write : node.info.writes)
			{
				const auto& resource = m_pool.resource(write);
				if (!std::holds_alternative<FGReferenceInfo>(resource.info))
					continue;

				const auto& refInfo = std::get<FGReferenceInfo>(resource.info);
				auto producer = producers.find(refInfo.handle);
				if (producer == producers.end())
					continue;

				if (FGNodeHandle != producer->second)
				{
					adjacency[FGNodeHandle.handle].emplace_back(producer->second);
					producers[refInfo.handle] = FGNodeHandle; // Update producer to the latest writer
				}
			}
		}

		// Link write -> read dependencies
		for (const auto& FGNodeHandle : m_nodes)
		{
			auto& node = m_pool.node(FGNodeHandle);
			for (auto& read : node.info.reads)
			{
				const auto& resource = m_pool.resource(read);
				if (!std::holds_alternative<FGReferenceInfo>(resource.info))
					continue;

				const auto& refInfo = std::get<FGReferenceInfo>(resource.info);
				auto producer = producers.find(refInfo.handle);
				if (producer == producers.end())
					continue;

				if (FGNodeHandle != producer->second)
				{
					adjacency[FGNodeHandle.handle].emplace_back(producer->second);
				}
			}
		}

		return adjacency;
	}

	auto FrameGraph::topologicalSort(const DependencyGraph& adjacency) -> std::vector<FGNodeHandle>
	{
		// Kahn's algorithm

		std::vector<size_t> inDegree(m_nodes.size(), 0);
		for (const auto& edges : adjacency)
		{
			for (const auto& target : edges)
			{
				inDegree[target.handle]++;
			}
		}

		std::queue<FGNodeHandle> queue;

		// Enqueue all nodes with no dependencies
		for (size_t i = 0; i < inDegree.size(); ++i)
		{
			if (inDegree[i] == 0)
			{
				queue.emplace(FGNodeHandle{ static_cast<uint32_t>(i) });
			}
		}

		std::vector<FGNodeHandle> sortedNodes;
		sortedNodes.reserve(m_nodes.size());

		while (!queue.empty())
		{
			auto nodeHandle = queue.front();
			queue.pop();

			sortedNodes.emplace_back(nodeHandle);
			for (const auto& neighbor : adjacency[nodeHandle.handle])
			{
				inDegree[neighbor.handle]--;
				if (inDegree[neighbor.handle] == 0)
				{
					queue.emplace(neighbor);
				}
			}
		}

		AGX_ASSERT_X(sortedNodes.size() == m_nodes.size(), "Cycle detected in FrameGraph!");

		// reverse to get correct order
		std::reverse(sortedNodes.begin(), sortedNodes.end());
		return sortedNodes;
	}

	void FrameGraph::generateBarriers()
	{
		struct ResourceUsage
		{
			FGResourceHandle resource{};
			VkImageLayout currentLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		};
		std::unordered_map<FGResourceHandle, ResourceUsage> resourceUsages;

		// Nodes are already sorted in execution order
		for (auto& nodeHandle : m_nodes)
		{
			auto& node = m_pool.node(nodeHandle);

			for (auto read : node.info.reads)
			{
				const auto& actualResource = m_pool.actualResource(read);
				const auto& lastUsage = resourceUsages[m_pool.actualHandle(read)];
				if (lastUsage.resource.isValid())
				{
					const auto& srcResource = m_pool.resource(lastUsage.resource);
					const auto& dstResource = m_pool.resource(read);
					addBarrier(node, srcResource, dstResource, actualResource);
				}
			}

			for (auto write : node.info.writes)
			{
				auto actualResourceHandle = m_pool.actualHandle(write);
				const auto& actualResource = m_pool.resource(actualResourceHandle);
				const auto& lastUsage = resourceUsages[actualResourceHandle];
				if (lastUsage.resource.isValid())
				{
					const auto& srcResource = m_pool.resource(lastUsage.resource);
					const auto& dstResource = m_pool.resource(write);
					addBarrier(node, srcResource, dstResource, actualResource);
				}

				// Update resource usage
				resourceUsages[actualResourceHandle] = ResourceUsage{
					.resource = write,
					.currentLayout = node.imageBarriers.empty() ? VK_IMAGE_LAYOUT_UNDEFINED : node.imageBarriers.back().newLayout,
				};
			}
		}
	}

	void FrameGraph::addBarrier(FGNode& node, const FGResource& srcResource, const FGResource& dstResource, const FGResource& actualResource)
	{
		auto srcAccessInfo = toAccessInfo(srcResource.usage);
		auto dstAccessInfo = toAccessInfo(dstResource.usage);

		node.srcStage |= srcAccessInfo.stage;
		node.dstStage |= dstAccessInfo.stage;

		if (std::holds_alternative<FGBufferInfo>(actualResource.info))
		{
			const auto& bufferInfo = std::get<FGBufferInfo>(actualResource.info);
			auto& buffer = m_pool.buffer(bufferInfo.handle);
			VkBufferMemoryBarrier barrier{
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				.srcAccessMask = srcAccessInfo.access,
				.dstAccessMask = dstAccessInfo.access,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.buffer = buffer.buffer(),
				.offset = 0,
				.size = VK_WHOLE_SIZE,
			};
			node.bufferBarriers.emplace_back(barrier);
		}
		else if (std::holds_alternative<FGTextureInfo>(actualResource.info))
		{
			const auto& textureInfo = std::get<FGTextureInfo>(actualResource.info);
			auto& texture = m_pool.texture(textureInfo.handle);
			VkImageMemoryBarrier barrier{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = srcAccessInfo.access,
				.dstAccessMask = dstAccessInfo.access,
				.oldLayout = srcAccessInfo.layout,
				.newLayout = dstAccessInfo.layout,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = texture.image(),
				.subresourceRange = VkImageSubresourceRange{
					.aspectMask = Tools::aspectFlags(texture.image().format()),
					.baseMipLevel = 0,
					.levelCount = texture.image().mipLevels(),
					.baseArrayLayer = 0,
					.layerCount = texture.image().layerCount(),
				}
			};
			node.imageBarriers.emplace_back(barrier);
		}
	}
}