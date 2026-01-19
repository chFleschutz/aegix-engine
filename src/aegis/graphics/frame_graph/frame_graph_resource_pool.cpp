#include "pch.h"
#include "frame_graph_resource_pool.h"

#include "core/globals.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/vulkan/vulkan_context.h"
#include "graphics/vulkan/vulkan_tools.h"

namespace Aegis::Graphics
{
	auto FGResourcePool::node(FGNodeHandle handle) -> FGNode&
	{
		AGX_ASSERT(handle.isValid());
		AGX_ASSERT(handle.handle < m_nodes.size());
		return m_nodes[handle.handle];
	}

	auto FGResourcePool::resource(FGResourceHandle handle) -> FGResource&
	{
		AGX_ASSERT(handle.isValid());
		AGX_ASSERT(handle.handle < m_resources.size());
		return m_resources[handle.handle];
	}

	auto FGResourcePool::actualResource(FGResourceHandle handle) -> FGResource&
	{
		return m_resources[actualHandle(handle).handle];
	}

	auto FGResourcePool::actualHandle(FGResourceHandle handle) -> FGResourceHandle
	{
		AGX_ASSERT(handle.isValid());
		const auto& res = resource(handle);
		if (auto info = std::get_if<FGReferenceInfo>(&res.info))
			return info->handle;
		return handle;
	}

	auto FGResourcePool::buffer(FGBufferHandle handle) -> BindlessMultiBuffer&
	{
		AGX_ASSERT(handle.isValid());
		AGX_ASSERT(handle.handle < m_buffers.size());
		return m_buffers[handle.handle];
	}

	auto FGResourcePool::buffer(FGResourceHandle handle) -> BindlessMultiBuffer&
	{
		auto& res = resource(actualHandle(handle));
		AGX_ASSERT_X(std::holds_alternative<FGBufferInfo>(res.info), "Resource is not a buffer");
		return buffer(std::get<FGBufferInfo>(res.info).handle);
	}

	auto FGResourcePool::texture(FGTextureHandle handle) -> Texture&
	{
		AGX_ASSERT(handle.isValid());
		AGX_ASSERT(handle.handle < m_textures.size());
		return m_textures[handle.handle];
	}

	auto FGResourcePool::texture(FGResourceHandle handle) -> Texture&
	{
		auto& res = resource(actualHandle(handle));
		AGX_ASSERT_X(std::holds_alternative<FGTextureInfo>(res.info), "Resource is not a texture");
		return texture(std::get<FGTextureInfo>(res.info).handle);
	}

	auto FGResourcePool::addNode(std::unique_ptr<FGRenderPass> pass) -> FGNodeHandle
	{
		auto nodeInfo = pass->info();
		m_nodes.emplace_back(nodeInfo, std::move(pass));
		return FGNodeHandle{ static_cast<uint32_t>(m_nodes.size() - 1) };
	}

	auto FGResourcePool::addBuffer(const std::string& name, FGResource::Usage usage, const FGBufferInfo& info) -> FGResourceHandle
	{
		m_resources.emplace_back(name, usage, info);
		return FGResourceHandle{ static_cast<uint32_t>(m_resources.size() - 1) };
	}

	auto FGResourcePool::addImage(const std::string& name, FGResource::Usage usage, const FGTextureInfo& info) -> FGResourceHandle
	{
		m_resources.emplace_back(name, usage, info);
		return FGResourceHandle{ static_cast<uint32_t>(m_resources.size() - 1) };
	}

	auto FGResourcePool::addReference(const std::string& name, FGResource::Usage usage) -> FGResourceHandle
	{
		m_resources.emplace_back(name, usage, FGReferenceInfo{});
		return FGResourceHandle{ static_cast<uint32_t>(m_resources.size() - 1) };
	}

	void FGResourcePool::resolveReferences()
	{
		for (auto& resource : m_resources)
		{
			if (!std::holds_alternative<FGReferenceInfo>(resource.info))
				continue;

			// Find the referenced resource by name
			auto& info = std::get<FGReferenceInfo>(resource.info);
			for (size_t i = 0; i < m_resources.size(); ++i)
			{
				const auto& other = m_resources[i];
				if (std::holds_alternative<FGReferenceInfo>(other.info))
					continue;

				if (other.name == resource.name)
				{
					info.handle = FGResourceHandle{ static_cast<uint32_t>(i) };
					break;
				}
			}

			if (!info.handle.isValid())
			{
				ALOG::fatal("Error: Unable to resolve reference for resource '{}'", resource.name);
				AGX_ASSERT_X(info.handle.isValid(), "Failed to resolve resource reference");
			}
		}
	}

	void FGResourcePool::createResources()
	{
		// Accumulate usage flags
		for (auto& res : m_resources)
		{
			FGResource* actualRes = &res;
			if (auto refInfo = std::get_if<FGReferenceInfo>(&res.info))
			{
				actualRes = &resource(refInfo->handle);
			}

			if (auto texInfo = std::get_if<FGTextureInfo>(&actualRes->info))
			{
				texInfo->usage |= FGResource::toImageUsage(res.usage);
			}
			else if (auto bufInfo = std::get_if<FGBufferInfo>(&actualRes->info))
			{
				bufInfo->usage |= FGResource::toBufferUsage(res.usage);
			}
		}

		// Create actual resources
		for (auto& res : m_resources)
		{
			if (std::holds_alternative<FGReferenceInfo>(res.info))
				continue;

			if (auto bufferInfo = std::get_if<FGBufferInfo>(&res.info))
			{
				bufferInfo->handle = createBuffer(*bufferInfo, res.name.c_str());
			}
			else if (auto textureInfo = std::get_if<FGTextureInfo>(&res.info))
			{
				textureInfo->handle = createImage(*textureInfo, res.name.c_str());
			}
		}
	}

	auto FGResourcePool::createBuffer(FGBufferInfo& info, const char* name) -> FGBufferHandle
	{
		// Typically uniform alignment is larger than storage buffer alignment -> prefer that
		auto alignment = VulkanContext::device().properties().limits.minStorageBufferOffsetAlignment;
		if (info.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
			alignment = VulkanContext::device().properties().limits.minUniformBufferOffsetAlignment;

		auto bufferCreateInfo = Buffer::CreateInfo{
			.instanceSize = info.size,
			.instanceCount = info.instanceCount,
			.usage = info.usage,
			.allocFlags = info.allocFlags,
			.minOffsetAlignment = alignment
		};
		m_buffers.emplace_back(bufferCreateInfo);

		Tools::vk::setDebugUtilsObjectName(m_buffers.back().buffer(), name);
		return FGBufferHandle{ static_cast<uint32_t>(m_buffers.size() - 1) };
	}

	auto FGResourcePool::createImage(FGTextureInfo& info, const char* name) -> FGTextureHandle
	{
		if (info.resizeMode == FGResizeMode::SwapChainRelative)
		{
			AGX_ASSERT_X(info.extent.width == 0 && info.extent.height == 0,
				"SwapChainRelative images must have initial extent of { 0, 0 }");
			info.extent = { Core::DEFAULT_WIDTH, Core::DEFAULT_HEIGHT };
		}

		auto textureCreateInfo = Texture::CreateInfo::texture2D(info.extent.width, info.extent.height, info.format);
		textureCreateInfo.image.usage = info.usage;
		textureCreateInfo.image.mipLevels = info.mipLevels;
		m_textures.emplace_back(textureCreateInfo);
		
		Tools::vk::setDebugUtilsObjectName(m_textures.back().image(), name);
		return FGTextureHandle{ static_cast<uint32_t>(m_textures.size() - 1) };
	}

	void FGResourcePool::resizeImages(uint32_t width, uint32_t height)
	{
		// Resize all swapchain-relative images
		auto cmd = VulkanContext::device().beginSingleTimeCommands();
		for (auto& res : m_resources)
		{
			if (!std::holds_alternative<FGTextureInfo>(res.info))
				continue;

			auto& info = std::get<FGTextureInfo>(res.info);
			if (info.resizeMode == FGResizeMode::SwapChainRelative)
			{
				auto& tex = m_textures[info.handle.handle];
				auto oldLayout = tex.image().layout();
				tex.resize({ width, height, 1 }, info.usage);
				tex.image().transitionLayout(cmd, oldLayout);
				info.extent = { width, height };
			}
		}
		VulkanContext::device().endSingleTimeCommands(cmd);

		// Update VkImage ref in barriers to new resized images
		for (auto& node : m_nodes)
		{
			AGX_ASSERT_X(node.imageBarriers.size() == node.accessedTextures.size(),
				"Mismatched image barriers and accessed textures count in FGNode");
			for (size_t i = 0; i < node.accessedTextures.size(); i++)
			{
				auto& tex = texture(node.accessedTextures[i]);
				auto& barrier = node.imageBarriers[i];
				barrier.image = tex.image();
			}
		}
	}
}
