#include "descriptors.h"

#include <cassert>
#include <stdexcept>

namespace Aegix::Graphics
{
	// *************** Descriptor Set Layout Builder *********************

	DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType,
		VkShaderStageFlags stageFlags, uint32_t count)
	{
		assert(m_bindings.count(binding) == 0 && "Binding already in use");

		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;
		m_bindings[binding] = layoutBinding;

		return *this;
	}

	std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const
	{
		return std::make_unique<DescriptorSetLayout>(m_device, m_bindings);
	}

	// *************** Descriptor Set Layout *********************

	DescriptorSetLayout::DescriptorSetLayout(VulkanDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
		: m_device{ device }, m_bindings{ bindings }
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
		for (auto& kv : bindings)
		{
			setLayoutBindings.push_back(kv.second);
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

		if (vkCreateDescriptorSetLayout(device.device(), &descriptorSetLayoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor set layout!");
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_device.device(), m_descriptorSetLayout, nullptr);
	}

	// *************** Descriptor Pool Builder *********************

	DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
	{
		m_poolSizes.push_back({ descriptorType, count });
		return *this;
	}

	DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags)
	{
		m_poolFlags = flags;
		return *this;
	}

	DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t count)
	{
		m_maxSets = count;
		return *this;
	}

	std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const
	{
		return std::make_unique<DescriptorPool>(m_device, m_maxSets, m_poolFlags, m_poolSizes);
	}

	// *************** Descriptor Pool *********************

	DescriptorPool::DescriptorPool(VulkanDevice& device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
		const std::vector<VkDescriptorPoolSize>& poolSizes)
		: m_device{ device }
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = poolFlags;

		if (vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor pool!");
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(m_device.device(), m_descriptorPool, nullptr);
	}

	bool DescriptorPool::allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		allocInfo.descriptorSetCount = 1;

		// Todo: Might want to create a "DescriptorPoolManager" class that handles this case, and builds
		// a new pool whenever an old pool fills up, see https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
		if (vkAllocateDescriptorSets(m_device.device(), &allocInfo, &descriptor) != VK_SUCCESS)
			return false;

		return true;
	}

	void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
	{
		vkFreeDescriptorSets(m_device.device(), m_descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
	}

	void DescriptorPool::resetPool()
	{
		vkResetDescriptorPool(m_device.device(), m_descriptorPool, 0);
	}

	// *************** Descriptor Writer *********************

	DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool)
		: m_setLayout{ setLayout }, m_pool{ pool }
	{
	}

	DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t binding, const VkDescriptorBufferInfo& bufferInfo)
	{
		assert(m_setLayout.m_bindings.contains(binding) && "Layout does not contain specified binding");

		auto& bindingDescription = m_setLayout.m_bindings[binding];
		assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

		m_bufferInfos.emplace_back(bufferInfo);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = &m_bufferInfos.back();
		write.descriptorCount = 1;

		m_writes.push_back(write);
		return *this;
	}

	DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, const VkDescriptorImageInfo& imageInfo)
	{
		assert(m_setLayout.m_bindings.contains(binding) && "Layout does not contain specified binding");

		auto& bindingDescription = m_setLayout.m_bindings[binding];
		assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

		m_imageInfos.emplace_back(imageInfo);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = bindingDescription.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = &m_imageInfos.back();
		write.descriptorCount = 1;

		m_writes.push_back(write);
		return *this;
	}

	void DescriptorWriter::build(VkDescriptorSet set)
	{
		for (auto& write : m_writes)
		{
			write.dstSet = set;
		}

		vkUpdateDescriptorSets(m_pool.m_device.device(), static_cast<uint32_t>(m_writes.size()), m_writes.data(), 0, nullptr);
	}

	DescriptorSet::Builder::Builder(VulkanDevice& device, DescriptorPool& pool, DescriptorSetLayout& setLayout)
		: m_device{ device }, m_pool{ pool }, m_setLayout{ setLayout }
	{
		m_writer.resize(SwapChain::MAX_FRAMES_IN_FLIGHT, DescriptorWriter(setLayout, pool));
	}

	DescriptorSet::Builder& DescriptorSet::Builder::addTexture(uint32_t binding, const Texture& texture)
	{
		for (auto& writer : m_writer)
		{
			writer.writeImage(binding, texture.descriptorImageInfo());
		}

		return *this;
	}

	std::unique_ptr<DescriptorSet> DescriptorSet::Builder::build()
	{
		auto set = std::make_unique<DescriptorSet>(m_pool, m_setLayout);

		for (size_t i = 0; i < m_writer.size(); i++)
		{
			m_writer[i].build(set->descriptorSet(i));
		}

		return set;
	}

	DescriptorSet::DescriptorSet(DescriptorPool& pool, DescriptorSetLayout& setLayout)
	{
		for (auto& set : m_descriptorSets)
		{
			assert(pool.allocateDescriptorSet(setLayout.descriptorSetLayout(), set) && "Failed to allocate descriptor set");
		}
	}
}
