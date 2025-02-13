#pragma once

#include "graphics/device.h"
#include "graphics/swap_chain.h"
#include "graphics/texture.h"
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
		DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);
		~DescriptorWriter() = default;

		DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

		void build(VkDescriptorSet set);

	private:
		DescriptorSetLayout& m_setLayout;
		DescriptorPool& m_pool;
		std::vector<VkWriteDescriptorSet> m_writes;
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
			Builder& addBuffer(uint32_t binding, const UniformBuffer<T>& buffer)
			{
				for (size_t i = 0; i < m_descriptorInfos.size(); i++)
				{
					m_descriptorInfos[i].bufferInfos.emplace_back(binding, buffer.descriptorBufferInfo(i));
				}
				return *this;
			}

			Builder& addTexture(uint32_t binding, const Texture& texture);
			Builder& addTexture(uint32_t binding, std::shared_ptr<Texture> texture);

			std::unique_ptr<DescriptorSet> build();

		private:
			struct DescriptorInfo
			{
				std::vector<std::pair<uint32_t, VkDescriptorBufferInfo>> bufferInfos;
				std::vector<std::pair<uint32_t, VkDescriptorImageInfo>> imageInfos;
			};

			VulkanDevice& m_device;
			DescriptorPool& m_pool;
			DescriptorSetLayout& m_setLayout;
			std::array<DescriptorInfo, SwapChain::MAX_FRAMES_IN_FLIGHT> m_descriptorInfos;
		};

		DescriptorSet(DescriptorPool& pool, DescriptorSetLayout& setLayout);
		~DescriptorSet() = default;

		const VkDescriptorSet& descriptorSet(int index) const { return m_descriptorSets[index]; }

	private:
		std::array<VkDescriptorSet, SwapChain::MAX_FRAMES_IN_FLIGHT> m_descriptorSets{};
	};
}
