#include "texture.h"

#include "core/engine.h"

namespace Aegix::Graphics
{
	auto Texture::create(const std::filesystem::path& texturePath, VkFormat format) -> std::shared_ptr<Texture>
	{
		auto texture = std::make_shared<Texture>(Engine::instance().device());
		texture->create2D(texturePath, format);
		return texture;
	}

	auto Texture::create(glm::vec4 color, VkFormat format) -> std::shared_ptr<Texture>
	{
		auto texture = std::make_shared<Texture>(Engine::instance().device());
		texture->create2D(1, 1, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
		texture->m_image.fillRGBA8(color);
		return texture;
	}

	Texture::Texture(VulkanDevice& device)
		: m_image{ device }, m_view{ device }, m_sampler{ device }
	{
	}

	void Texture::create2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels)
	{
		m_image.create({ width, height, 1 }, format, usage, mipLevels);
		m_view.create(m_image, 0, m_image.mipLevels(), 0, m_image.layerCount());
		m_sampler.create({});
	}

	void Texture::create2D(const std::filesystem::path& path, VkFormat format)
	{
		m_image.create(path, format);
		m_view.create(m_image, 0, m_image.mipLevels(), 0, m_image.layerCount());
		m_sampler.create({});
	}

	void Texture::createCube(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels)
	{
		m_image.create({ width, height, 1 }, format, usage, mipLevels, 6);
		m_view.create(m_image, 0, m_image.mipLevels(), 0, m_image.layerCount());
		m_sampler.create({});
	}

	void Texture::resize(VkExtent3D newSize, VkImageUsageFlags usage)
	{
		m_image.resize(newSize, usage);
		m_view.create(m_image, 0, m_image.mipLevels(), 0, m_image.layerCount());
	}
}
