#pragma once

#include "core/asset.h"
#include "graphics/resources/image.h"
#include "graphics/resources/image_view.h"
#include "graphics/resources/sampler.h"

#include <glm/glm.hpp>

namespace Aegix::Graphics
{
	class Texture : public Core::Asset
	{
	public:
		struct CreateInfo
		{
			static auto texture2D(uint32_t width, uint32_t height, VkFormat format) -> CreateInfo;
			static auto cubeMap(uint32_t size, VkFormat format) -> CreateInfo;

			Image::CreateInfo image;
			ImageView::CreateInfo view;
			Sampler::CreateInfo sampler;
		};

		//static auto create(const std::filesystem::path& texturePath) -> std::shared_ptr<Texture>;
		//static auto create(const std::filesystem::path& texturePath, VkFormat format) -> std::shared_ptr<Texture>;
		//static auto create(glm::vec4 color, VkFormat format) -> std::shared_ptr<Texture>;

		static auto loadFromFile(const std::filesystem::path& file, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM) -> std::shared_ptr<Texture>;
		static auto loadTextur2D(const std::filesystem::path& file, VkFormat format) -> std::shared_ptr<Texture>;
		static auto loadCubemap(const std::filesystem::path& file) -> std::shared_ptr<Texture>;

		static auto solidColor(glm::vec4 color) -> std::shared_ptr<Texture>;
		static auto solidColorCube(glm::vec4 color) -> std::shared_ptr<Texture>;

		static auto irradianceMap(const std::shared_ptr<Texture>& skybox) -> std::shared_ptr<Texture>;
		static auto prefilteredMap(const std::shared_ptr<Texture>& skybox) -> std::shared_ptr<Texture>;
		static auto BRDFLUT() -> std::shared_ptr<Texture>;

		Texture() = default;
		Texture(const CreateInfo& info);
		Texture(const Texture&) = delete;
		Texture(Texture&&) noexcept = default;
		~Texture() = default;

		auto operator=(const Texture&) -> Texture& = delete;
		auto operator=(Texture&&) noexcept -> Texture& = default;
		
		[[nodiscard]] auto image() -> Image& { return m_image; }
		[[nodiscard]] auto image() const -> const Image& { return m_image; }
		[[nodiscard]] auto view() -> ImageView& { return m_view; }
		[[nodiscard]] auto view() const -> const ImageView& { return m_view; }
		[[nodiscard]] auto sampler() -> Sampler& { return m_sampler; }
		[[nodiscard]] auto sampler() const -> const Sampler& { return m_sampler; }
		[[nodiscard]] auto extent() const -> VkExtent3D { return m_image.extent(); }
		[[nodiscard]] auto extent2D() const -> VkExtent2D { return VkExtent2D{ m_image.width(), m_image.height() }; }

		[[nodiscard]] auto descriptorImageInfo() const -> VkDescriptorImageInfo
		{
			return descriptorImageInfo(m_image.layout());
		}

		[[nodiscard]] auto descriptorImageInfo(VkImageLayout layoutOverride) const -> VkDescriptorImageInfo
		{
			return VkDescriptorImageInfo{
				.sampler = m_sampler.sampler(),
				.imageView = m_view.imageView(),
				.imageLayout = layoutOverride,
			};
		}

		//void create2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels = Image::CreateInfo::CALCULATE_MIP_LEVELS);
		//void create2D(const Image::CreateInfo& config, const Sampler::CreateInfo& samplerConfig);
		//void create2D(const std::filesystem::path& path, VkFormat format);

		//void createCube(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels = Image::CreateInfo::CALCULATE_MIP_LEVELS);
		//void createCube(const std::filesystem::path& path);

		void resize(VkExtent3D newSize, VkImageUsageFlags usage);
		
	private:
		Image m_image;
		ImageView m_view;
		Sampler m_sampler;
	};
}
