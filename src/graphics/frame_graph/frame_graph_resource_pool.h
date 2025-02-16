#pragma once

#include "graphics/texture.h"

#include <string>

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
		//Texture texture;
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
			m_textures.emplace_back(name/*, Texture{ device, desc.width, desc.height, desc.format, desc.usage }*/);
			return static_cast<FrameGraphResourceID>(m_textures.size() - 1);
		}

		const FrameGraphTexture& getTexture(FrameGraphResourceID id) const
		{
			return m_textures.at(id);
		}

	private:
		std::vector<FrameGraphTexture> m_textures;
	};
}