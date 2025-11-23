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

		// TODO: Debug
		// Print nodes for debugging
		std::cout << "FrameGraph Nodes:\n";
		for (size_t i = 0; i < m_nodes.size(); ++i)
		{
			auto& node = m_pool.node(m_nodes[i]);
			std::cout << "  Node [" << i << "]: " << node.info.name << "\n";
		}

		{
			auto graph = buildDependencyGraph();

			// TODO: Debug
			// Print dependency graph for debugging
			std::cout << "FrameGraph Dependency Graph:\n";
			for (size_t i = 0; i < graph.size(); ++i)
			{
				std::cout << "  Node [" << i << "] -> ";
				for (const auto& neighbor : graph[i])
				{
					std::cout << neighbor.handle << " ";
				}
				std::cout << "\n";
			}

			m_nodes = topologicalSort(graph);
		}

		// TODO: Debug
		// Print sorted nodes for debugging
		std::cout << "FrameGraph Sorted Nodes:\n";
		for (size_t i = 0; i < m_nodes.size(); i++)
		{
			auto& node = m_pool.node(m_nodes[i]);
			std::cout << "  Node [" << i << "]: " << node.info.name << "\n";
		}


		// TODO: Compute resource lifetimes for aliasing

		m_pool.createResources();


		// TODO: Debug
		// Print resources for debugging
		std::cout << "FrameGraph Resources:\n";
		for (size_t i = 0; i < m_pool.resources().size(); i++)
		{
			auto& resource = m_pool.resources()[i];
			std::cout << "  Resource [" << i << "]: " << resource.name;
			if (std::holds_alternative<FGReferenceInfo>(resource.info))
				std::cout << " (Reference)";
			std::cout << "\n";
		}

		generateBarriers();

		// TODO: Debug
		// Print barriers for debugging
		std::cout << "FrameGraph Barriers:\n";
		for (size_t i = 0; i < m_nodes.size(); i++)
		{
			auto& node = m_pool.node(m_nodes[i]);
			std::cout << "  Node [" << i << "]: Buffer: " << node.bufferBarriers.size() << 
				" - Image: " << node.imageBarriers.size() << "\n";
		}
	}

	void FrameGraph::execute(const FrameInfo& frameInfo)
	{
		AGX_PROFILE_FUNCTION();

		for (auto nodeHandle : m_nodes)
		{
			auto& node = m_pool.node(nodeHandle);

			Tools::vk::cmdBeginDebugUtilsLabel(frameInfo.cmd, node.info.name.c_str());
			{
				placeBarriers(frameInfo.cmd, node);
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
		std::unordered_map<FGResourceHandle, FGResourceHandle> lastUsage;

		auto barrierLambda = [this, &lastUsage](FGNode& node, FGResourceHandle handle)
		{
			auto actualResourceHandle = m_pool.actualHandle(handle);
			auto lastUsageHandle = lastUsage[actualResourceHandle];
			if (lastUsageHandle.isValid())
			{
				generateBarrier(node, lastUsageHandle, handle, actualResourceHandle);
			}
			lastUsage[actualResourceHandle] = handle;
		};

		for (auto nodeHandle : m_nodes)
		{
			auto& node = m_pool.node(nodeHandle);
			node.imageBarriers.clear();
			node.bufferBarriers.clear();
			node.srcStage = 0;
			node.dstStage = 0;

			for (auto readHandle : node.info.reads)
			{
				barrierLambda(node, readHandle);
			}

			for (auto writeHandle : node.info.writes)
			{
				barrierLambda(node, writeHandle);
			}
		}
	}

	void FrameGraph::generateBarrier(FGNode& node, FGResourceHandle srcHandle, FGResourceHandle dstHandle, FGResourceHandle actualHandle)
	{
		const auto& srcResource = m_pool.resource(srcHandle);
		const auto& dstResource = m_pool.resource(dstHandle);
		const auto& actualResource = m_pool.resource(actualHandle);

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
			node.accessedTextures.emplace_back(textureInfo.handle);
		}
	}

	void FrameGraph::placeBarriers(VkCommandBuffer cmd, const FGNode& node)
	{
		Tools::vk::cmdPipelineBarrier(cmd, node.srcStage, node.dstStage,
			node.bufferBarriers, node.imageBarriers);

		// Images track their layout internally, so update it after the barrier
		for (size_t i = 0; i < node.accessedTextures.size(); i++)
		{
			auto& texture = m_pool.texture(node.accessedTextures[i]);
			texture.image().setLayout(node.imageBarriers[i].newLayout);
		}
	}
}