#pragma once

#include "graphics/descriptors.h"
#include "graphics/systems/render_system.h"
#include "graphics/texture.h"
#include "graphics/uniform_buffer.h"

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace Aegix::Graphics
{
	using FrameGraphResourceID = uint32_t;

	enum class ResizePolicy
	{
		Fixed,
		SwapchainRelative
	};

	struct FrameGraphTexture
	{
		std::string name;
		Texture texture;
		VkImageUsageFlags usage;
		ResizePolicy resizePolicy;
	};

	struct RenderStage
	{
		enum class Type
		{
			Geometry,
			Count
		};

		std::vector<std::unique_ptr<RenderSystem>> renderSystems;
		std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
		std::unique_ptr<DescriptorSet> descriptorSet;
		std::unique_ptr<UniformBuffer> ubo;
	};

	class FrameGraphResourcePool
	{
	public:
		FrameGraphResourcePool() = default;
		FrameGraphResourcePool(const FrameGraphResourcePool&) = delete;
		FrameGraphResourcePool(FrameGraphResourcePool&&) = delete;
		~FrameGraphResourcePool() = default;

		FrameGraphResourcePool& operator=(const FrameGraphResourcePool&) = delete;
		FrameGraphResourcePool& operator=(FrameGraphResourcePool&&) = delete;

		FrameGraphResourceID addTexture(VulkanDevice& device, const std::string& name, uint32_t width, uint32_t height, 
			VkFormat format, VkImageUsageFlags usage, ResizePolicy resizePolicy)
		{
			m_textures.emplace_back(name, Texture{ device, width, height, format, usage }, usage, resizePolicy);
			return static_cast<FrameGraphResourceID>(m_textures.size() - 1);
		}

		template<typename T>
			requires std::is_base_of_v<RenderSystem, T>
		RenderSystem& addRenderSystem(VulkanDevice& device, RenderStage::Type stageType)
		{
			auto& stage = renderStage(stageType);

			// Check if rendersystem type already exists
			for (auto& system : stage.renderSystems)
			{
				if (dynamic_cast<T*>(system.get()))
					return *system;
			}

			stage.renderSystems.emplace_back(std::make_unique<T>(device, *stage.descriptorSetLayout));
			return *stage.renderSystems.back();
		}

		auto texture(FrameGraphResourceID id) -> FrameGraphTexture& { return m_textures[static_cast<size_t>(id)]; }
		auto texture(FrameGraphResourceID id) const -> const FrameGraphTexture& { return m_textures[static_cast<size_t>(id)]; }
		auto textures() -> std::vector<FrameGraphTexture>& { return m_textures; }

		auto renderStage(RenderStage::Type type) -> RenderStage& { return m_renderStages[static_cast<size_t>(type)]; }
		auto renderStage(RenderStage::Type type) const -> const RenderStage& { return m_renderStages[static_cast<size_t>(type)]; }

	private:
		std::vector<FrameGraphTexture> m_textures;
		std::array<RenderStage, static_cast<size_t>(RenderStage::Type::Count)> m_renderStages;
	};
}