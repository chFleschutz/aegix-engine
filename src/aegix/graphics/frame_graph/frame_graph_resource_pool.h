#pragma once

#include "graphics/frame_graph/frame_graph_node.h"
#include "graphics/frame_graph/frame_graph_resources.h"
#include "graphics/resources/buffer.h"
#include "graphics/resources/texture.h"

namespace Aegix::Graphics
{
	/// @brief Holds all resources and nodes for a frame graph
	class FGResourcePool
	{
		friend class FrameGraph;

	public:
		[[nodiscard]] auto nodes() const -> const std::vector<FGNode>& { return m_nodes; }
		[[nodiscard]] auto resources() const -> const std::vector<FGResource>& { return m_resources; }
		[[nodiscard]] auto buffers() const -> const std::vector<Buffer>& { return m_buffers; }
		[[nodiscard]] auto textures() const -> const std::vector<Texture>& { return m_textures; }

		[[nodiscard]] auto node(FGNodeHandle handle) -> FGNode&;
		[[nodiscard]] auto resource(FGResourceHandle handle) -> FGResource&;
		[[nodiscard]] auto actualResource(FGResourceHandle handle) -> FGResource&;
		[[nodiscard]] auto actualHandle(FGResourceHandle handle) -> FGResourceHandle;
		[[nodiscard]] auto buffer(FGBufferHandle handle) -> Buffer&;
		[[nodiscard]] auto buffer(FGResourceHandle handle) -> Buffer&;
		[[nodiscard]] auto texture(FGTextureHandle handle) -> Texture&;
		[[nodiscard]] auto texture(FGResourceHandle handle) -> Texture&;

		auto addNode(std::unique_ptr<FGRenderPass> pass) -> FGNodeHandle;
		auto addBuffer(const std::string& name, FGResource::Usage usage, const FGBufferInfo& info) -> FGResourceHandle;
		auto addImage(const std::string& name, FGResource::Usage usage, const FGTextureInfo& info) -> FGResourceHandle;
		auto addReference(const std::string& name, FGResource::Usage usage) -> FGResourceHandle;
		
	private:
		void resolveReferences();
		void createResources();
		auto createBuffer(FGBufferInfo& info) -> FGBufferHandle;
		auto createImage(FGTextureInfo& info) -> FGTextureHandle;
		void resizeImages(uint32_t width, uint32_t height);

		std::vector<FGNode> m_nodes;
		std::vector<FGResource> m_resources;
		std::vector<Buffer> m_buffers;
		std::vector<Texture> m_textures;
	};
}