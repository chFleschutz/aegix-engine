#include "pch.h"

#include "texture.h"

#include "engine.h"
#include "graphics/vulkan_tools.h"

#include <stb_image.h>

namespace Aegix::Graphics
{
	auto Texture::create(const std::filesystem::path& texturePath) -> std::shared_ptr<Texture>
	{
		if (!std::filesystem::exists(texturePath))
		{
			ALOG::fatal("Texture file does not exist: '{}'", texturePath.string());
			AGX_ASSERT_X(false, "Texture file does not exist");
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

	auto Texture::createIrradiance(const std::shared_ptr<Texture>& skybox) -> std::shared_ptr<Texture>
	{
		auto& device = Engine::instance().device();

		// Create irradiance map
		constexpr uint32_t irradianceSize = 32;
		auto irradiance = std::make_shared<Texture>(device);
		irradiance->createCube(irradianceSize, irradianceSize, VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);

		// Create pipeline resources
		auto descriptorSetLayout = DescriptorSetLayout::Builder{ device }
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		auto& pool = Engine::instance().renderer().globalPool();
		auto descriptorSet = std::make_shared<DescriptorSet>(pool, *descriptorSetLayout);
		DescriptorWriter{ *descriptorSetLayout }
			.writeImage(0, skybox->descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
			.writeImage(1, irradiance->descriptorImageInfo(VK_IMAGE_LAYOUT_GENERAL))
			.build(descriptorSet->descriptorSet(0));

		auto pipeline = Pipeline::ComputeBuilder{}
			.addDescriptorSetLayout(*descriptorSetLayout)
			.setShaderStage(SHADER_DIR "irradiance_convolution.comp.spv")
			.buildUnique();

		// Convert skybox to irradiance map
		VkCommandBuffer cmd = device.beginSingleTimeCommands();
		Tools::vk::cmdBeginDebugUtilsLabel(cmd, "Irradiance Convolution");
		{
			skybox->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			irradiance->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);

			pipeline->bind(cmd);
			descriptorSet->bind(cmd, pipeline->layout(), 0, VK_PIPELINE_BIND_POINT_COMPUTE);

			constexpr uint32_t groupSize = 16;
			uint32_t width = irradiance->image().width();
			uint32_t height = irradiance->image().height();
			vkCmdDispatch(cmd, (width + groupSize - 1) / groupSize, (height + groupSize) / groupSize, 6);

			irradiance->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		Tools::vk::cmdEndDebugUtilsLabel(cmd);
		device.endSingleTimeCommands(cmd);

		return irradiance;
	}

	auto Texture::createPrefiltered(const std::shared_ptr<Texture>& skybox) -> std::shared_ptr<Texture>
	{
		auto& device = Engine::instance().device();

		// Create prefiltered map
		constexpr uint32_t prefilteredSize = 128;
		constexpr uint32_t mipLevelCount = 5;
		auto prefiltered = std::make_shared<Texture>(device);
		prefiltered->createCube(prefilteredSize, prefilteredSize, VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, mipLevelCount);

		// Create pipeline resources
		auto descriptorSetLayout = DescriptorSetLayout::Builder{ device }
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		struct PushConstants
		{
			float roughness = 0.0f;
			float envResolution = 512.0f;
		} pushConstants;
		pushConstants.envResolution = static_cast<float>(skybox->image().width());

		auto pipeline = Pipeline::ComputeBuilder{}
			.addDescriptorSetLayout(*descriptorSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(pushConstants))
			.setShaderStage(SHADER_DIR "prefilter_environment.comp.spv")
			.buildUnique();

		// Create image views for mip levels
		auto& pool = Engine::instance().renderer().globalPool();
		std::vector<DescriptorSet> descriptorSets;
		descriptorSets.reserve(mipLevelCount);
		std::vector<ImageView> mipViews;
		mipViews.reserve(mipLevelCount);
		for (uint32_t i = 0; i < mipLevelCount; ++i)
		{
			auto& view = mipViews.emplace_back(device);
			view.create(prefiltered->image(), ImageView::Config{
				.baseMipLevel = i,
				.levelCount = 1,
				.baseLayer = 0,
				.layerCount = 6,
				.viewType = VK_IMAGE_VIEW_TYPE_CUBE
				});

			descriptorSets.emplace_back(pool, *descriptorSetLayout);
			DescriptorWriter{ *descriptorSetLayout }
				.writeImage(0, skybox->descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
				.writeImage(1, VkDescriptorImageInfo{ VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_GENERAL })
				.build(descriptorSets.back().descriptorSet(0));
		}

		// Convert skybox to prefiltered map
		VkCommandBuffer cmd = device.beginSingleTimeCommands();
		Tools::vk::cmdBeginDebugUtilsLabel(cmd, "Prefilter Environment");
		{
			skybox->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			prefiltered->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);
			pipeline->bind(cmd);
			
			for (uint32_t mip = 0; mip < mipLevelCount; ++mip)
			{
				descriptorSets[mip].bind(cmd, pipeline->layout(), 0, VK_PIPELINE_BIND_POINT_COMPUTE);

				pushConstants.roughness = static_cast<float>(mip) / static_cast<float>(mipLevelCount - 1);
				vkCmdPushConstants(cmd, pipeline->layout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pushConstants), &pushConstants);

				constexpr uint32_t groupSize = 16;
				uint32_t width = prefiltered->image().width() >> mip;
				uint32_t height = prefiltered->image().height() >> mip;
				vkCmdDispatch(cmd, (width + groupSize - 1) / groupSize, (height + groupSize - 1) / groupSize, 6);
			}

			prefiltered->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		Tools::vk::cmdEndDebugUtilsLabel(cmd);
		device.endSingleTimeCommands(cmd);

		return prefiltered;
	}

	auto Texture::createBRDFLUT() -> std::shared_ptr<Texture>
	{
		auto& device = Engine::instance().device();

		// Create BRDF LUT
		constexpr uint32_t lutSize = 512;
		Image::Config imgageConfig{
			.format = VK_FORMAT_R16G16_SFLOAT,
			.extent = { lutSize, lutSize, 1 },
			.mipLevels = 1,
			.layerCount = 1,
			.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		};
		Sampler::Config samplerConfig{
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		};
		auto lut = std::make_shared<Texture>(device);
		lut->create2D(imgageConfig, samplerConfig);

		// Create pipeline resources
		auto descriptorSetLayout = DescriptorSetLayout::Builder{ device }
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();
		auto& pool = Engine::instance().renderer().globalPool();
		auto descriptorSet = std::make_shared<DescriptorSet>(pool, *descriptorSetLayout);
		DescriptorWriter{ *descriptorSetLayout }
			.writeImage(0, lut->descriptorImageInfo(VK_IMAGE_LAYOUT_GENERAL))
			.build(descriptorSet->descriptorSet(0));
		auto pipeline = Pipeline::ComputeBuilder{}
			.addDescriptorSetLayout(*descriptorSetLayout)
			.setShaderStage(SHADER_DIR "brdf_lut.comp.spv")
			.buildUnique();

		// Convert skybox to irradiance map
		VkCommandBuffer cmd = device.beginSingleTimeCommands();
		Tools::vk::cmdBeginDebugUtilsLabel(cmd, "BRDF LUT Generation");
		{
			lut->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);

			pipeline->bind(cmd);
			descriptorSet->bind(cmd, pipeline->layout(), 0, VK_PIPELINE_BIND_POINT_COMPUTE);

			constexpr uint32_t groupSize = 16;
			vkCmdDispatch(cmd, (lutSize + groupSize - 1) / groupSize, (lutSize + groupSize - 1) / groupSize, 1);

			lut->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		Tools::vk::cmdEndDebugUtilsLabel(cmd);
		device.endSingleTimeCommands(cmd);

		return lut;
	}



	Texture::Texture(VulkanDevice& device)
		: m_image{ device }, m_view{ device }, m_sampler{ device }
	{
	}

	void Texture::create2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels)
	{
		m_image.create({ width, height, 1 }, format, usage, mipLevels);
		m_view.create(m_image, 0, m_image.mipLevels(), 0, m_image.layerCount());
		m_sampler.create(Sampler::Config{
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.maxLod = static_cast<float>(m_image.mipLevels() - 1)
			});
	}

	void Texture::create2D(const Image::Config& config, const Sampler::Config& samplerConfig)
	{
		m_image.create(config);
		m_view.create(m_image, 0, m_image.mipLevels(), 0, m_image.layerCount());
		m_sampler.create(samplerConfig);
	}

	void Texture::create2D(const std::filesystem::path& path, VkFormat format)
	{
		m_image.create(path, format);
		m_view.create(m_image, 0, m_image.mipLevels(), 0, m_image.layerCount());
		m_sampler.create(Sampler::Config{
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.maxLod = static_cast<float>(m_image.mipLevels() - 1)
			});
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

		m_sampler.create(Sampler::Config{
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.maxLod = static_cast<float>(m_image.mipLevels() - 1)
			});
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
			AGX_ASSERT_X(false, "Failed to load image");
		}

		// Upload data to staging buffer
		auto& device = m_image.device();
		VkDeviceSize imageSize = 4 * sizeof(float) * static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height);
		Buffer stagingBuffer{ device, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT };
		stagingBuffer.singleWrite(pixels);

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

		auto pipeline = Pipeline::ComputeBuilder{}
			.addDescriptorSetLayout(*descriptorSetLayout)
			.setShaderStage(SHADER_DIR "equirect_to_cube.comp.spv")
			.buildUnique();

		// Convert spherical image to cubemap
		VkCommandBuffer cmd = device.beginSingleTimeCommands();
		Tools::vk::cmdBeginDebugUtilsLabel(cmd, "Equirectangular to Cubemap");
		{
			spherialImage.image().copyFrom(cmd, stagingBuffer);
			m_image.transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);

			pipeline->bind(cmd);
			descriptorSet->bind(cmd, pipeline->layout(), 0, VK_PIPELINE_BIND_POINT_COMPUTE);

			constexpr uint32_t groupSize = 16;
			uint32_t groupCountX = (width + groupSize - 1) / groupSize;
			uint32_t groupCountY = (height + groupSize - 1) / groupSize;
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
