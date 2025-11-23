#include "pch.h"
#include "frame_graph_resource_pool.h"

#include "core/globals.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/vulkan/vulkan_context.h"

namespace Aegix::Graphics
{
	auto FGResourcePool::node(FGNodeHandle handle) -> FGNode&
	{
		assert(handle.isValid());
		assert(handle.handle < m_nodes.size());
		return m_nodes[handle.handle];
	}


	auto FGResourcePool::resource(FGResourceHandle handle) -> FGResource&
	{
		assert(handle.isValid());
		assert(handle.handle < m_resources.size());
		return m_resources[handle.handle];
	}

	auto FGResourcePool::actualResource(FGResourceHandle handle) -> FGResource&
	{
		return m_resources[actualHandle(handle).handle];
	}

	auto FGResourcePool::actualHandle(FGResourceHandle handle) -> FGResourceHandle
	{
		assert(handle.isValid());
		const auto& res = resource(handle);
		if (auto info = std::get_if<FGReferenceInfo>(&res.info))
			return info->handle;
		return handle;
	}

	auto FGResourcePool::buffer(FGBufferHandle handle) -> Buffer&
	{
		assert(handle.isValid());
		assert(handle.handle < m_buffers.size());
		return m_buffers[handle.handle];
	}

	auto FGResourcePool::buffer(FGResourceHandle handle) -> Buffer&
	{
		auto& res = resource(actualHandle(handle));
		AGX_ASSERT_X(std::holds_alternative<FGBufferInfo>(res.info), "Resource is not a buffer");
		return buffer(std::get<FGBufferInfo>(res.info).handle);
	}

	auto FGResourcePool::texture(FGTextureHandle handle) -> Texture&
	{
		assert(handle.isValid());
		assert(handle.handle < m_textures.size());
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
				bufferInfo->handle = createBuffer(*bufferInfo);
			}
			else if (auto textureInfo = std::get_if<FGTextureInfo>(&res.info))
			{
				textureInfo->handle = createImage(*textureInfo);
			}
		}
	}

	auto FGResourcePool::createBuffer(FGBufferInfo& info) -> FGBufferHandle
	{
		auto bufferCreateInfo = Buffer::CreateInfo{
			.instanceSize = info.size,
			.instanceCount = 1,
			.usage = info.usage,
		};
		m_buffers.emplace_back(bufferCreateInfo);
		return FGBufferHandle{ static_cast<uint32_t>(m_buffers.size() - 1) };
	}

	auto FGResourcePool::createImage(FGTextureInfo& info) -> FGTextureHandle
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

		return FGTextureHandle{ static_cast<uint32_t>(m_textures.size() - 1) };
	}

	void FGResourcePool::resizeImages(uint32_t width, uint32_t height)
	{
		for (auto& res : m_resources)
		{
			if (!std::holds_alternative<FGTextureInfo>(res.info))
				continue;

			auto& info = std::get<FGTextureInfo>(res.info);
			if (info.resizeMode == FGResizeMode::SwapChainRelative)
			{
				m_textures[info.handle.handle].resize({ width, height, 1 }, info.usage);
				info.extent = { width, height };
			}
		}
	}
}
