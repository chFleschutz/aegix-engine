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

	struct FrameGraphTexture
	{
		struct Desc
		{
			uint32_t width;
			uint32_t height;
			VkFormat format;
			VkImageUsageFlags usage;
		};

		std::string name;
		Texture texture;
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

		FrameGraphResourceID addTexture(VulkanDevice& device, const std::string& name, const FrameGraphTexture::Desc& desc)
		{
			m_textures.emplace_back(name, Texture{ device, desc.width, desc.height, desc.format, desc.usage });
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

		auto renderStage(RenderStage::Type type) -> RenderStage& { return m_renderStages[static_cast<size_t>(type)]; }
		auto renderStage(RenderStage::Type type) const -> const RenderStage& { return m_renderStages[static_cast<size_t>(type)]; }

	private:
		std::vector<FrameGraphTexture> m_textures;
		std::array<RenderStage, static_cast<size_t>(RenderStage::Type::Count)> m_renderStages;
	};
}