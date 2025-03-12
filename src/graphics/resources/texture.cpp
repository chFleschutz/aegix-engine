#include "texture.h"

#include "core/engine.h"
#include "graphics/vulkan_tools.h"

#include <stb_image.h>

namespace Aegix::Graphics
{
	auto Texture::create(const std::filesystem::path& texturePath) -> std::shared_ptr<Texture>
	{
		if (!std::filesystem::exists(texturePath))
		{
			ALOG::fatal("Texture file does not exist: '{}'", texturePath.string());
			assert(false && "Texture file does not exist");
		}

		if (texturePath.extension() == ".hdr")
		{
			auto texture = std::make_shared<Texture>(Engine::instance().device());
			texture->createCube(texturePath);
			return texture;
		}

		return create(texturePath, VK_FORMAT_R8G8B8A8_UNORM);
	}

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
		Image::Config imageConfig{
			.format = format,
			.extent = { width, height, 1 },
			.mipLevels = mipLevels,
			.layerCount = 6,
			.usage = usage,
			.imageType = VK_IMAGE_TYPE_2D,
			.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
		};
		m_image.create(imageConfig);

		ImageView::Config viewConfig{
			.baseMipLevel = 0,
			.levelCount = m_image.mipLevels(),
			.baseLayer = 0,
			.layerCount = m_image.layerCount(),
			.viewType = VK_IMAGE_VIEW_TYPE_CUBE
		};
		m_view.create(m_image, viewConfig);

		m_sampler.create({});
	}

	void Texture::createCube(const std::filesystem::path& path)
	{
		// HDR environment maps are stored as equirectangular images (longitude/latitude 2D image)
		// To convert it to a cubemap, the image is sampled in a compute shader and written to the cubemap 

		int width = 0;
		int height = 0;
		int channels = 0;
		auto pixels = stbi_loadf(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (!pixels)
		{
			ALOG::fatal("Failed to load image: '{}'", path.string());
			assert(false && "Failed to load image");
		}

		// Upload data to staging buffer
		auto& device = m_image.device();
		VkDeviceSize imageSize = 4 * sizeof(float) * static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height);
		Buffer stagingBuffer{ device, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
		stagingBuffer.map();
		stagingBuffer.writeToBuffer(pixels);
		stagingBuffer.unmap();

		stbi_image_free(pixels);

		// Create spherical image
		Texture spherialImage{ device };
		spherialImage.create2D(static_cast<uint32_t>(width), static_cast<uint32_t>(height), VK_FORMAT_R32G32B32A32_SFLOAT,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
		
		// Create cubemap image
		uint32_t cubeSize = width / 4; // Cubemap needs 4 horizontal faces
		createCube(cubeSize, cubeSize, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		// Create pipeline resources
		auto descriptorSetLayout = DescriptorSetLayout::Builder{ device }
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		auto& pool = Engine::instance().renderer().globalPool();
		auto descriptorSet = std::make_shared<DescriptorSet>(pool, *descriptorSetLayout);

		DescriptorWriter{ *descriptorSetLayout }
			.writeImage(0, spherialImage.descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
			.writeImage(1, descriptorImageInfo(VK_IMAGE_LAYOUT_GENERAL))
			.build(descriptorSet->descriptorSet(0));

		auto pipelineLayout = PipelineLayout::Builder{ device }
			.addDescriptorSetLayout(*descriptorSetLayout)
			.build();

		auto pipeline = Pipeline::ComputeBuilder{ device, *pipelineLayout }
			.setShaderStage(SHADER_DIR "equirect_to_cube.comp.spv")
			.build();

		// Convert spherical image to cubemap
		VkCommandBuffer cmd = device.beginSingleTimeCommands();
		Tools::vk::cmdBeginDebugUtilsLabel(cmd, "Equirectangular to Cubemap");
		{
			spherialImage.image().copyFrom(cmd, stagingBuffer);
			m_image.transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);

			pipeline->bind(cmd);
			descriptorSet->bind(cmd, *pipelineLayout, 0, VK_PIPELINE_BIND_POINT_COMPUTE);

			uint32_t groupCountX = (width + width - 1) / 16;
			uint32_t groupCountY = (height + height - 1) / 16;
			vkCmdDispatch(cmd, groupCountX, groupCountY, 6);

			m_image.generateMipmaps(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		Tools::vk::cmdEndDebugUtilsLabel(cmd);
		device.endSingleTimeCommands(cmd);
	}

	void Texture::resize(VkExtent3D newSize, VkImageUsageFlags usage)
	{
		m_image.resize(newSize, usage);
		m_view.create(m_image, 0, m_image.mipLevels(), 0, m_image.layerCount());
	}
}
