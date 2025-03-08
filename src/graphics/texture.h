#pragma once

#include "graphics/buffer.h"
#include "graphics/device.h"
#include "graphics/swap_chain.h"

#include <glm/glm.hpp>

#include <filesystem>

namespace Aegix::Graphics
{
	class Texture;

	class ImageView
	{
	public:
		explicit ImageView(VulkanDevice& device);
		ImageView(const Texture& texture, uint32_t baseMipLevel, uint32_t levelCount);
		ImageView(const ImageView&) = delete;
		ImageView(ImageView&& other) noexcept;
		~ImageView();

		auto operator=(const ImageView&) -> ImageView& = delete;
		auto operator=(ImageView&& other) noexcept -> ImageView&;

		operator VkImageView() const { return m_imageView; }
		[[nodiscard]] auto imageView() const -> VkImageView { return m_imageView; }

	private:
		void destroy();

		VulkanDevice& m_device;
		VkImageView m_imageView = VK_NULL_HANDLE;
	};


	class Sampler
	{
	public:
		Sampler(VulkanDevice& device, VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode, bool anisotropy);
		Sampler(const Sampler&) = delete;
		Sampler(Sampler&& other) noexcept;
		~Sampler();

		auto operator=(const Sampler&) -> Sampler& = delete;
		auto operator=(Sampler&& other) noexcept -> Sampler&;

		operator VkSampler() const { return m_sampler; }
		[[nodiscard]] auto sampler() const -> VkSampler { return m_sampler; }

	private:
		void destroy();

		VulkanDevice& m_device;
		VkSampler m_sampler = VK_NULL_HANDLE;
	};


	/// @brief Represents a Texture on the gpu and wraps a VkImage, VkDeviceMemory and VkImageView
	class Texture
	{
		friend class ImageView;

	public:
		struct Config
		{
			static constexpr uint32_t CALCULATE_MIP_LEVELS = 0;

			// Image settings
			VkExtent2D extent = { 1, 1 };
			VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
			VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
			uint32_t mipLevels = 1;

			// Sampler settings
			VkFilter magFilter = VK_FILTER_LINEAR;
			VkFilter minFilter = VK_FILTER_LINEAR;
			VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			bool anisotropy = true;
		};

		static auto create(const std::filesystem::path& texturePath, VkFormat format) -> std::shared_ptr<Texture>;
		static auto create(VkExtent2D extent, glm::vec4 color, VkFormat format) -> std::shared_ptr<Texture>;

		Texture(VulkanDevice& device, const Config& config);

		/// @brief Creates a texture from a file, supports: jpeg, png, tga, bmp, psd, gif, hdr, pic, pnm (see: https://github.com/nothings/stb/blob/master/stb_image.h)
		/// @note The texture will be in VK_LAYOUT_SHADER_READ_ONLY_OPTIMAL layout
		Texture(VulkanDevice& device, const std::filesystem::path& texturePath, VkFormat format);

		/// @brief Creates a texture with a single color
		/// @note The texture will be in VK_LAYOUT_SHADER_READ_ONLY_OPTIMAL layout
		Texture(VulkanDevice& device, uint32_t width, uint32_t height, const glm::vec4& color, VkFormat format);

		/// @brief Creates an empty texture (primarily for render targets)
		/// @note The texture will be in VK_LAYOUT_UNDEFINED layout
		Texture(VulkanDevice& device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);

		/// @brief Wraps an existing image and view 
		Texture(VulkanDevice& device, const SwapChain& swapChain);

		Texture(const Texture&) = delete;
		Texture(Texture&& other) noexcept;
		~Texture();

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&& other) noexcept;

		[[nodiscard]] auto extent() const -> VkExtent2D{ return m_extent; }
		[[nodiscard]] auto width() const -> uint32_t { return m_extent.width; }
		[[nodiscard]] auto height() const -> uint32_t { return m_extent.height; }
		[[nodiscard]] auto mipLevels() const -> uint32_t { return m_mipLevels; }
		[[nodiscard]] auto format() const -> VkFormat { return m_format; }
		[[nodiscard]] auto layout() const -> VkImageLayout { return m_layout; }
		[[nodiscard]] auto image() const -> VkImage { return m_image; }
		[[nodiscard]] auto imageView() const -> VkImageView { return m_imageView; }
		[[nodiscard]] auto sampler() const -> VkSampler { return m_sampler; }

		[[nodiscard]] auto descriptorImageInfo() const -> VkDescriptorImageInfo;

		/// @brief Returns a VkImageMemoryBarrier for transitioning the image to the new layout
		/// @note The internal layout will be changed to the new layout, make sure to submit the barrier to a command buffer
		[[nodiscard]] auto transitionLayoutDeferred(VkImageLayout newLayout) -> VkImageMemoryBarrier;

		void fill(const glm::vec4& color);
		void fill(const void* data, VkDeviceSize size);
		void fill(const Buffer& buffer);

		void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);
		void transitionLayout(VkImageLayout newLayout);

		void resize(uint32_t width, uint32_t height, VkImageUsageFlags usage);

		/// @brief Updates the texture with a new image and view from the swap chain (USE WITH CAUTION)
		/// @note The old image and view will NOT be destroyed 
		void update(const SwapChain& swapChain);

		/// @brief Updates the texture with a new image and view (USE WITH CAUTION)
		/// @note The old image and view will NOT be destroyed
		void update(VkImage image, VkImageView imageView, VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED);

		/// @brief Fills the textures mip levels by downsampling the base level
		/// @note Afterwards the texture will be in 'finalLayout'
		void generateMipmaps(VkCommandBuffer commandBuffer, VkImageLayout finalLayout);

	private:
		void createImage(const Config& config);
		void createImageView(const Config& config);
		void createSampler(const Config& config);
		void destroy();

		VulkanDevice& m_device;

		VkFormat m_format;
		VkExtent2D m_extent;
		uint32_t m_mipLevels = 1;
		VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImage m_image = VK_NULL_HANDLE;
		VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
		VkImageView m_imageView = VK_NULL_HANDLE;
		VkSampler m_sampler = VK_NULL_HANDLE;
	};
}
