#pragma once

#include "graphics/device.h"
#include "graphics/globals.h"
#include "graphics/resources/texture.h"
#include "graphics/uniform_buffer.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace Aegix::Graphics
{
	class DescriptorSetLayout
	{
	public:
		class Builder
		{
		public:
			Builder(VulkanDevice& device) : m_device{ device } {}

			Builder& addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags,
				uint32_t count = 1);
			std::unique_ptr<DescriptorSetLayout> build() const;

		private:
			VulkanDevice& m_device;
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings{};
		};

		DescriptorSetLayout(VulkanDevice& lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
		~DescriptorSetLayout();
		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

		operator VkDescriptorSetLayout() const { return m_descriptorSetLayout; }
		VkDescriptorSetLayout descriptorSetLayout() const { return m_descriptorSetLayout; }

	private:
		VulkanDevice& m_device;
		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;

		friend class DescriptorWriter;
	};



	class DescriptorPool
	{
	public:
		class Builder
		{
		public:
			Builder(VulkanDevice& device) : m_device{ device } {}

			Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
			Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
			Builder& setMaxSets(uint32_t count);
			std::unique_ptr<DescriptorPool> build() const;

		private:
			VulkanDevice& m_device;
			std::vector<VkDescriptorPoolSize> m_poolSizes{};
			uint32_t m_maxSets = 1000;
			VkDescriptorPoolCreateFlags m_poolFlags = 0;
		};

		DescriptorPool(VulkanDevice& device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
			const std::vector<VkDescriptorPoolSize>& poolSizes);
		~DescriptorPool();
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;

		operator VkDescriptorPool() const { return m_descriptorPool; }
		VkDescriptorPool descriptorPool() const { return m_descriptorPool; }

		bool allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

		void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

		void resetPool();

	private:
		VulkanDevice& m_device;
		VkDescriptorPool m_descriptorPool;

		friend class DescriptorWriter;
	};



	class DescriptorWriter
	{
	public:
		DescriptorWriter(DescriptorSetLayout& setLayout);
		DescriptorWriter(const DescriptorWriter&) = delete;
		DescriptorWriter(DescriptorWriter&&) = default;
		~DescriptorWriter() = default;

		DescriptorWriter& operator=(const DescriptorWriter&) = delete;
		DescriptorWriter& operator=(DescriptorWriter&&) = default;

		DescriptorWriter& writeImage(uint32_t binding, const Texture& texture);
		DescriptorWriter& writeImage(uint32_t binding, const Texture& texture, VkImageLayout layoutOverride);
		DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo imageInfo);

		DescriptorWriter& writeBuffer(uint32_t binding, const Buffer& buffer);
		DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo bufferInfo);

		void build(VkDescriptorSet set);

	private:
		DescriptorSetLayout& m_setLayout;
		std::vector<std::pair<uint32_t, VkDescriptorImageInfo>> m_imageInfos;
		std::vector<std::pair<uint32_t, VkDescriptorBufferInfo>> m_bufferInfos;
	};


	class DescriptorSet
	{
	public:
		class Builder
		{
		public:
			Builder(VulkanDevice& device, DescriptorPool& pool, DescriptorSetLayout& setLayout);
			Builder(const Builder&) = delete;
			~Builder() = default;

			Builder& operator=(const Builder&) = delete;

			template<typename T>
			Builder& addBuffer(uint32_t binding, const UniformBufferData<T>& buffer)
			{
				for (size_t i = 0; i < m_writer.size(); i++)
				{
					m_writer[i].writeBuffer(binding, buffer.descriptorBufferInfo(i));
				}
				return *this;
			}

			Builder& addBuffer(uint32_t binding, const UniformBuffer& buffer);
			Builder& addTexture(uint32_t binding, const Texture& texture);
			Builder& addTexture(uint32_t binding, std::shared_ptr<Texture> texture);

			std::unique_ptr<DescriptorSet> build();

		private:
			VulkanDevice& m_device;
			DescriptorPool& m_pool;
			DescriptorSetLayout& m_setLayout;
			std::vector<DescriptorWriter> m_writer;
		};

		DescriptorSet(DescriptorPool& pool, DescriptorSetLayout& setLayout);
		~DescriptorSet() = default;

		const VkDescriptorSet& operator[](int index) const { return m_descriptorSets[index]; }
		const VkDescriptorSet& descriptorSet(int index) const { return m_descriptorSets[index]; }

		void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int index, 
			VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

	private:
		std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_descriptorSets{};
	};
}
