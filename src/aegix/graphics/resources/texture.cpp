#include "pch.h"
#include "texture.h"

#include "engine.h"
#include "graphics/descriptors.h"
#include "graphics/pipeline.h"
#include "graphics/resources/buffer.h"
#include "graphics/vulkan/vulkan_tools.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace Aegix::Graphics
{
	auto Texture::CreateInfo::texture2D(uint32_t width, uint32_t height, VkFormat format) -> CreateInfo
	{
		return CreateInfo{
			.image = Image::CreateInfo{
				.format = format,
				.extent = VkExtent3D{ width, height, 1 },
				.mipLevels = Image::CreateInfo::CALCULATE_MIP_LEVELS,
			},
		};
	}

	auto Texture::CreateInfo::cubeMap(uint32_t size, VkFormat format) -> CreateInfo
	{
		return CreateInfo{
			.image = Image::CreateInfo{
				.format = format,
				.extent = VkExtent3D{ size, size, 1 },
				.mipLevels = Image::CreateInfo::CALCULATE_MIP_LEVELS,
				.layerCount = 6,
				.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				.imageType = VK_IMAGE_TYPE_2D,
				.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			},
			.view = ImageView::CreateInfo{
				.viewType = VK_IMAGE_VIEW_TYPE_CUBE,
			},
		};
	}



	auto Texture::loadFromFile(const std::filesystem::path& texturePath, VkFormat format) -> std::shared_ptr<Texture>
	{
		if (!std::filesystem::exists(texturePath))
		{
			ALOG::fatal("Texture file does not exist: '{}'", texturePath.string());
			AGX_ASSERT_X(false, "Texture file does not exist");
		}

		if (texturePath.extension() == ".hdr")
		{
			return Texture::loadCubemap(texturePath);
		}

		return Texture::loadTextur2D(texturePath, format);
	}

	auto Texture::loadTextur2D(const std::filesystem::path& file, VkFormat format) -> std::shared_ptr<Texture>
	{
		int stbWidth = 0;
		int stbHeight = 0;
		int stbChannels = 0;
		auto pixels = stbi_load(file.string().c_str(), &stbWidth, &stbHeight, &stbChannels, STBI_rgb_alpha);
		if (!pixels)
		{
			ALOG::fatal("Failed to load image: '{}'", file.string());
			AGX_ASSERT_X(false, "Failed to load image");
		}

		uint32_t width = static_cast<uint32_t>(stbWidth);
		uint32_t height = static_cast<uint32_t>(stbHeight);
		VkDeviceSize imageSize = 4 * static_cast<VkDeviceSize>(stbWidth) * static_cast<VkDeviceSize>(stbHeight);

		Texture::CreateInfo info = Texture::CreateInfo::texture2D(width, height, format);
		auto texture = std::make_shared<Texture>(info);
		texture->image().upload(pixels, imageSize);

		stbi_image_free(pixels);
		return texture;
	}

	auto Texture::loadCubemap(const std::filesystem::path& file) -> std::shared_ptr<Texture>
	{
		// HDR environment maps are stored as equirectangular images (longitude/latitude 2D image)
		// To convert it to a cubemap, the image is sampled in a compute shader and written to the cubemap 

		int stbWidth = 0;
		int stbHeight = 0;
		int stbChannels = 0;
		auto pixels = stbi_loadf(file.string().c_str(), &stbWidth, &stbHeight, &stbChannels, STBI_rgb_alpha);
		if (!pixels)
		{
			ALOG::fatal("Failed to load image: '{}'", file.string());
			AGX_ASSERT_X(false, "Failed to load image");
		}

		// Upload data to staging buffer
		VkDeviceSize imageSize = 4 * sizeof(float) * static_cast<VkDeviceSize>(stbWidth) * static_cast<VkDeviceSize>(stbHeight);
		Buffer stagingBuffer = Buffer::createStagingBuffer(imageSize);
		stagingBuffer.singleWrite(pixels);

		stbi_image_free(pixels);

		// Create spherical image
		uint32_t width = static_cast<uint32_t>(stbWidth);
		uint32_t height = static_cast<uint32_t>(stbHeight);
		auto sphericalInfo = Texture::CreateInfo::texture2D(width, height, VK_FORMAT_R32G32B32A32_SFLOAT);
		sphericalInfo.image.mipLevels = 1;
		Texture spherialImage{ sphericalInfo };

		// Create cubemap image
		uint32_t cubeSize = width / 4; // Cubemap needs 4 horizontal faces
		auto cubeInfo = Texture::CreateInfo::cubeMap(cubeSize, VK_FORMAT_R16G16B16A16_SFLOAT);
		cubeInfo.image.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		auto cubeMap = std::make_shared<Texture>(cubeInfo);

		// Create pipeline resources
		auto descriptorSetLayout = DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		DescriptorSet descriptorSet{ descriptorSetLayout };

		DescriptorWriter{ descriptorSetLayout }
			.writeImage(0, spherialImage.descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
			.writeImage(1, cubeMap->descriptorImageInfo(VK_IMAGE_LAYOUT_GENERAL))
			.update(descriptorSet);

		auto pipeline = Pipeline::ComputeBuilder{}
			.addDescriptorSetLayout(descriptorSetLayout)
			.setShaderStage(SHADER_DIR "ibl/equirect_to_cube.comp.spv")
			.build();

		// Convert spherical image to cubemap
		VkCommandBuffer cmd = VulkanContext::device().beginSingleTimeCommands();
		Tools::vk::cmdBeginDebugUtilsLabel(cmd, "Equirectangular to Cubemap");
		{
			spherialImage.image().copyFrom(cmd, stagingBuffer);
			cubeMap->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);

			pipeline.bind(cmd);
			pipeline.bindDescriptorSet(cmd, 0, descriptorSet);

			constexpr uint32_t groupSize = 16;
			uint32_t groupCountX = (width + groupSize - 1) / groupSize;
			uint32_t groupCountY = (height + groupSize - 1) / groupSize;
			vkCmdDispatch(cmd, groupCountX, groupCountY, 6);

			cubeMap->image().generateMipmaps(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		Tools::vk::cmdEndDebugUtilsLabel(cmd);
		VulkanContext::device().endSingleTimeCommands(cmd);

		return cubeMap;
	}

	auto Texture::solidColor(glm::vec4 color) -> std::shared_ptr<Texture>
	{
		auto info = Texture::CreateInfo::texture2D(1, 1, VK_FORMAT_R32G32B32A32_SFLOAT);
		auto texture = std::make_shared<Texture>(info);
		texture->image().upload(&color, sizeof(glm::vec4));
		return texture;
	}

	auto Texture::solidColorCube(glm::vec4 color) -> std::shared_ptr<Texture>
	{
		std::array<glm::vec4, 6> colors{ color, color, color, color, color, color };
		auto info = Texture::CreateInfo::cubeMap(1, VK_FORMAT_R32G32B32A32_SFLOAT);
		auto texture = std::make_shared<Texture>(info);
		texture->image().upload(&colors, sizeof(colors));
		return texture;
	}

	auto Texture::irradianceMap(const std::shared_ptr<Texture>& skybox) -> std::shared_ptr<Texture>
	{
		// Create irradiance map
		constexpr uint32_t irradianceSize = 32;

		auto textureInfo = Texture::CreateInfo::cubeMap(irradianceSize, VK_FORMAT_R16G16B16A16_SFLOAT);
		textureInfo.image.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		textureInfo.image.mipLevels = 1;
		auto irradiance = std::make_shared<Texture>(textureInfo);

		// Create pipeline resources
		auto descriptorSetLayout = DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		DescriptorSet descriptorSet{ descriptorSetLayout };
		DescriptorWriter{ descriptorSetLayout }
			.writeImage(0, skybox->descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
			.writeImage(1, irradiance->descriptorImageInfo(VK_IMAGE_LAYOUT_GENERAL))
			.update(descriptorSet);

		auto pipeline = Pipeline::ComputeBuilder{}
			.addDescriptorSetLayout(descriptorSetLayout)
			.setShaderStage(SHADER_DIR "ibl/irradiance_convolution.comp.spv")
			.build();

		// Convert skybox to irradiance map
		VkCommandBuffer cmd = VulkanContext::device().beginSingleTimeCommands();
		Tools::vk::cmdBeginDebugUtilsLabel(cmd, "Irradiance Convolution");
		{
			skybox->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			irradiance->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);

			pipeline.bind(cmd);
			pipeline.bindDescriptorSet(cmd, 0, descriptorSet);

			constexpr uint32_t groupSize = 16;
			uint32_t width = irradiance->image().width();
			uint32_t height = irradiance->image().height();
			vkCmdDispatch(cmd, (width + groupSize - 1) / groupSize, (height + groupSize) / groupSize, 6);

			irradiance->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		Tools::vk::cmdEndDebugUtilsLabel(cmd);
		VulkanContext::device().endSingleTimeCommands(cmd);

		return irradiance;
	}

	auto Texture::prefilteredMap(const std::shared_ptr<Texture>& skybox) -> std::shared_ptr<Texture>
	{
		// Create prefiltered map
		constexpr uint32_t prefilteredSize = 128;
		constexpr uint32_t mipLevelCount = 5;

		auto textureInfo = Texture::CreateInfo::cubeMap(prefilteredSize, VK_FORMAT_R16G16B16A16_SFLOAT);
		textureInfo.image.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		textureInfo.image.mipLevels = mipLevelCount;
		auto prefiltered = std::make_shared<Texture>(textureInfo);

		// Create pipeline resources
		auto descriptorSetLayout = DescriptorSetLayout::Builder{}
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
			.addDescriptorSetLayout(descriptorSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(pushConstants))
			.setShaderStage(SHADER_DIR "ibl/prefilter_environment.comp.spv")
			.build();

		// Create image views for mip levels
		std::vector<ImageView> mipViews;
		mipViews.reserve(mipLevelCount);

		std::vector<DescriptorSet> descriptorSets;
		descriptorSets.reserve(mipLevelCount);

		for (uint32_t i = 0; i < mipLevelCount; ++i)
		{
			ImageView::CreateInfo viewInfo{
				.baseMipLevel = i,
				.levelCount = 1,
				.baseLayer = 0,
				.layerCount = 6,
				.viewType = VK_IMAGE_VIEW_TYPE_CUBE,
			};
			mipViews.emplace_back(viewInfo, prefiltered->image());

			descriptorSets.emplace_back(descriptorSetLayout);
			DescriptorWriter{ descriptorSetLayout }
				.writeImage(0, skybox->descriptorImageInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
				.writeImage(1, VkDescriptorImageInfo{ VK_NULL_HANDLE, mipViews[i], VK_IMAGE_LAYOUT_GENERAL })
				.update(descriptorSets[i]);
		}

		// Convert skybox to prefiltered map
		VkCommandBuffer cmd = VulkanContext::device().beginSingleTimeCommands();
		Tools::vk::cmdBeginDebugUtilsLabel(cmd, "Prefilter Environment");
		{
			skybox->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			prefiltered->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);
			pipeline.bind(cmd);

			for (uint32_t mip = 0; mip < mipLevelCount; ++mip)
			{
				pipeline.bindDescriptorSet(cmd, 0, descriptorSets[mip]);

				pushConstants.roughness = static_cast<float>(mip) / static_cast<float>(mipLevelCount - 1);
				pipeline.pushConstants(cmd, VK_SHADER_STAGE_COMPUTE_BIT, &pushConstants, sizeof(pushConstants));

				constexpr uint32_t groupSize = 16;
				uint32_t width = prefiltered->image().width() >> mip;
				uint32_t height = prefiltered->image().height() >> mip;
				vkCmdDispatch(cmd, (width + groupSize - 1) / groupSize, (height + groupSize - 1) / groupSize, 6);
			}

			prefiltered->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		Tools::vk::cmdEndDebugUtilsLabel(cmd);
		VulkanContext::device().endSingleTimeCommands(cmd);

		return prefiltered;
	}

	auto Texture::BRDFLUT() -> std::shared_ptr<Texture>
	{
		// Create BRDF LUT
		constexpr uint32_t lutSize = 512;

		auto textureInfo = Texture::CreateInfo::texture2D(lutSize, lutSize, VK_FORMAT_R16G16_SFLOAT);
		textureInfo.image.mipLevels = 1;
		textureInfo.image.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		textureInfo.sampler.addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		auto lut = std::make_shared<Texture>(textureInfo);

		// Create pipeline resources
		auto descriptorSetLayout = DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		DescriptorSet descriptorSet{ descriptorSetLayout };
		DescriptorWriter{ descriptorSetLayout }
			.writeImage(0, lut->descriptorImageInfo(VK_IMAGE_LAYOUT_GENERAL))
			.update(descriptorSet);

		auto pipeline = Pipeline::ComputeBuilder{}
			.addDescriptorSetLayout(descriptorSetLayout)
			.setShaderStage(SHADER_DIR "ibl/brdf_lut.comp.spv")
			.build();

		// Convert skybox to irradiance map
		VkCommandBuffer cmd = VulkanContext::device().beginSingleTimeCommands();
		Tools::vk::cmdBeginDebugUtilsLabel(cmd, "BRDF LUT Generation");
		{
			lut->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL);

			pipeline.bind(cmd);
			pipeline.bindDescriptorSet(cmd, 0, descriptorSet);

			constexpr uint32_t groupSize = 16;
			vkCmdDispatch(cmd, (lutSize + groupSize - 1) / groupSize, (lutSize + groupSize - 1) / groupSize, 1);

			lut->image().transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		Tools::vk::cmdEndDebugUtilsLabel(cmd);
		VulkanContext::device().endSingleTimeCommands(cmd);

		return lut;
	}

	Texture::Texture(const CreateInfo& info) :
		m_image{ info.image },
		m_view{ info.view, m_image },
		m_sampler{ info.sampler, m_image.mipLevels() }
	{
		auto& bindlessSet = Engine::renderer().bindlessDescriptorSet();
		if (info.image.usage & VK_IMAGE_USAGE_SAMPLED_BIT)
			m_sampledHandle = bindlessSet.allocateSampledImage(*this);

		if (info.image.usage & VK_IMAGE_USAGE_STORAGE_BIT)
			m_storageHandle = bindlessSet.allocateStorageImage(*this);
	}

	Texture::~Texture()
	{
		Engine::renderer().bindlessDescriptorSet().freeHandle(m_storageHandle);
		Engine::renderer().bindlessDescriptorSet().freeHandle(m_sampledHandle);
	}

	void Texture::resize(VkExtent3D newSize, VkImageUsageFlags usage)
	{
		// TODO: Rework this, does not work for all textures (e.g. cube maps)

		m_image.resize(newSize, usage);

		ImageView::CreateInfo viewInfo{
			.baseMipLevel = 0,
			.levelCount = m_image.mipLevels(),
			.baseLayer = 0,
			.layerCount = m_image.layerCount(),
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
		};
		m_view = ImageView{ viewInfo, m_image };
	}
}
