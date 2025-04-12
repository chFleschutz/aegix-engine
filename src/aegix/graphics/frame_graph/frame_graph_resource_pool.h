#pragma once

#include "core/globals.h"
#include "graphics/frame_graph/render_stage.h"
#include "graphics/resources/buffer.h"
#include "graphics/resources/texture.h"

#include <variant>

namespace Aegix::Graphics
{
	class FrameGraphRenderPass;

	struct FrameGraphResourceHandle
	{
		uint32_t id{ Core::INVALID_HANDLE };

		constexpr auto operator<=>(const FrameGraphResourceHandle&) const = default;
	};

	struct FrameGraphNodeHandle
	{
		uint32_t id{ Core::INVALID_HANDLE };

		constexpr auto operator<=>(const FrameGraphNodeHandle&) const = default;
	};


	// FrameGraphResource --------------------------------------------------------

	enum class FrameGraphResourceType
	{
		Buffer,
		Texture,
		Reference
	};

	enum class ResizePolicy
	{
		Fixed,
		SwapchainRelative
	};

	enum class FrameGraphResourceUsage
	{
		None,
		Sampled,
		ColorAttachment,
		DepthStencilAttachment,
		Compute,
		TransferSrc,
		TransferDst,
		Present
	};

	struct FrameGraphResourceBufferInfo
	{
		// TODO: Add buffer info
	};

	struct FrameGraphResourceTextureInfo
	{
		VkFormat format;
		VkExtent2D extent;
		ResizePolicy resizePolicy = ResizePolicy::Fixed;
		VkImageUsageFlags usage = 0;
		uint32_t mipLevels = 1;
	};

	using FrameGraphResourceInfo = std::variant<FrameGraphResourceBufferInfo, FrameGraphResourceTextureInfo>;

	struct FrameGraphResourceCreateInfo
	{
		std::string name;
		FrameGraphResourceType type;
		FrameGraphResourceUsage usage;
		FrameGraphResourceInfo info;
	};

	struct FrameGraphResource
	{
		static constexpr FrameGraphResourceHandle INVALID_HANDLE{ std::numeric_limits<uint32_t>::max() };

		std::string name;
		FrameGraphResourceType type;
		FrameGraphResourceUsage usage;
		FrameGraphResourceInfo info;
		FrameGraphResourceHandle handle;
		FrameGraphNodeHandle producer;
	};


	// FrameGraphNode ------------------------------------------------------------

	struct FrameGraphNodeCreateInfo
	{
		std::string name;
		std::vector<FrameGraphResourceHandle> inputs;
		std::vector<FrameGraphResourceHandle> outputs;
	};

	struct FrameGraphNode
	{
		static constexpr FrameGraphNodeHandle INVALID_HANDLE{ std::numeric_limits<uint32_t>::max() };

		std::string name;
		std::unique_ptr<FrameGraphRenderPass> pass;
		std::vector<FrameGraphResourceHandle> inputs;
		std::vector<FrameGraphResourceHandle> outputs;
		std::vector<FrameGraphNodeHandle> edges;
	};


	// FrameGraphResourcePool ----------------------------------------------------

	/// @brief Holds all resources and nodes for a frame graph
	class FrameGraphResourcePool
	{
		friend class FrameGraph;

	public:
		FrameGraphResourcePool() = default;
		FrameGraphResourcePool(const FrameGraphResourcePool&) = delete;
		FrameGraphResourcePool(FrameGraphResourcePool&&) = delete;
		~FrameGraphResourcePool() = default;

		FrameGraphResourcePool& operator=(const FrameGraphResourcePool&) = delete;
		FrameGraphResourcePool& operator=(FrameGraphResourcePool&&) = delete;
		
		/// @brief Returns the node for the given handle
		[[nodiscard]] auto node(FrameGraphNodeHandle handle) -> FrameGraphNode&;
		[[nodiscard]] auto node(FrameGraphNodeHandle handle) const -> const FrameGraphNode&;

		/// @brief Returns the resource for the given handle
		[[nodiscard]] auto resource(FrameGraphResourceHandle handle) -> FrameGraphResource&;
		[[nodiscard]] auto resource(FrameGraphResourceHandle handle) const -> const FrameGraphResource&;

		/// @brief Returns the resource for the given handle and resolves any references
		[[nodiscard]] auto finalResource(FrameGraphResourceHandle handle) -> FrameGraphResource&;
		[[nodiscard]] auto finalResource(FrameGraphResourceHandle handle) const -> const FrameGraphResource&;

		/// @brief Returns the texture for the given handle (must be a texture resource)
		[[nodiscard]] auto texture(FrameGraphResourceHandle resourceHandle) -> Texture&;
		[[nodiscard]] auto texture(FrameGraphResourceHandle resourceHandle) const -> const Texture&;

		[[nodiscard]] auto resources() -> std::vector<FrameGraphResource>& { return m_resources; }
		[[nodiscard]] auto nodes() -> std::vector<FrameGraphNode>& { return m_nodes; }

		auto addNode(std::unique_ptr<FrameGraphRenderPass> pass) -> FrameGraphNodeHandle;
		auto addResource(const FrameGraphResourceCreateInfo& createInfo, FrameGraphNodeHandle producer) -> FrameGraphResourceHandle;
		auto addResource(Texture texture, const FrameGraphResourceCreateInfo& createInfo, FrameGraphNodeHandle producer) -> FrameGraphResourceHandle;

		/// @brief Adds an existing texture as a resource
		auto addExternalResource(Texture texture, const FrameGraphResourceCreateInfo& createInfo) -> FrameGraphResourceHandle;

		template<typename T>
			requires std::is_base_of_v<RenderSystem, T>
		RenderSystem& addRenderSystem(RenderStage::Type stageType)
		{
			auto& stage = renderStage(stageType);

			// Check if rendersystem type already exists
			for (auto& system : stage.renderSystems)
			{
				if (dynamic_cast<T*>(system.get()))
					return *system;
			}

			stage.renderSystems.emplace_back(std::make_unique<T>(*stage.descriptorSetLayout));
			return *stage.renderSystems.back();
		}

		auto renderStage(RenderStage::Type type) -> RenderStage& { return m_renderStages[static_cast<size_t>(type)]; }
		auto renderStage(RenderStage::Type type) const -> const RenderStage& { return m_renderStages[static_cast<size_t>(type)]; }

		/// @brief For all reference resources, resolve the handle to the actual resource
		void resolveReferences();
		void createResources();

		void resizeImages(uint32_t width, uint32_t height);

	private:
		void createTexture(FrameGraphResource& resource);
		void createBuffer(FrameGraphResource& resource);

		std::vector<FrameGraphNode> m_nodes;
		std::vector<FrameGraphResource> m_resources;
		std::vector<Texture> m_textures;
		std::vector<Buffer> m_buffers;

		std::array<RenderStage, static_cast<size_t>(RenderStage::Type::Count)> m_renderStages;
	};


	// FrameGraphResourceBuilder ------------------------------------------------

	class FrameGraphResourceBuilder
	{
	public:
		FrameGraphResourceBuilder(FrameGraphResourcePool& pool, FrameGraphNodeHandle node)
			: m_pool{ pool }, m_node{ node } {
		}

		auto add(const FrameGraphResourceCreateInfo& createInfo) -> FrameGraphResourceHandle
		{
			return m_pool.addResource(createInfo, m_node);
		}

		auto add(Texture texture, const FrameGraphResourceCreateInfo& createInfo) -> FrameGraphResourceHandle
		{
			return m_pool.addResource(std::move(texture), createInfo, m_node);
		}

	private:
		FrameGraphResourcePool& m_pool;
		FrameGraphNodeHandle m_node;
	};
}