#include "pch.h"

#include "descriptors.h"

#include "graphics/vulkan_tools.h"

#include <cassert>

namespace Aegix::Graphics
{
	// DescriptorSetLayout Builder -----------------------------------------------------

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



	// DescriptorSetLayout -----------------------------------------------------

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

		VK_CHECK(vkCreateDescriptorSetLayout(device.device(), &descriptorSetLayoutInfo, nullptr, &m_descriptorSetLayout))
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_device.device(), m_descriptorSetLayout, nullptr);
	}



	// DescriptorPool Builder -----------------------------------------------------

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



	// DescriptorPool -----------------------------------------------------

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

		VK_CHECK(vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &m_descriptorPool))
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



	// DescriptorWriter -----------------------------------------------------

	DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout)
		: m_setLayout{ setLayout }
	{
	}

	DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, const Texture& texture)
	{
		m_imageInfos.emplace_back(binding, texture.descriptorImageInfo());
		return *this;
	}

	DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, const Texture& texture, VkImageLayout layoutOverride)
	{
		m_imageInfos.emplace_back(binding, texture.descriptorImageInfo(layoutOverride));
		return *this;
	}

	DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo imageInfo)
	{
		m_imageInfos.emplace_back(binding, std::move(imageInfo));
		return *this;
	}

	DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t binding, const Buffer& buffer)
	{
		m_bufferInfos.emplace_back(binding, VkDescriptorBufferInfo{ buffer.buffer(), 0, VK_WHOLE_SIZE });
		return *this;
	}

	DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo bufferInfo)
	{
		m_bufferInfos.emplace_back(binding, std::move(bufferInfo));
		return *this;
	}

	void DescriptorWriter::build(VkDescriptorSet set)
	{
		std::vector<VkWriteDescriptorSet> writes;
		writes.reserve(m_bufferInfos.size() + m_imageInfos.size());

		for (const auto& [binding, info] : m_imageInfos)
		{
			auto& bindingDesc = m_setLayout.m_bindings[binding];
			assert(bindingDesc.descriptorCount == 1 && "Cannot write multiple images to a single descriptor");

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorType = bindingDesc.descriptorType;
			write.dstBinding = binding;
			write.pImageInfo = &info;
			write.descriptorCount = 1;
			write.dstSet = set;
			writes.emplace_back(write);
		}

		for (const auto& [binding, info] : m_bufferInfos)
		{
			auto& bindingDesc = m_setLayout.m_bindings[binding];
			assert(bindingDesc.descriptorCount == 1 && "Cannot write multiple images to a single descriptor");

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorType = bindingDesc.descriptorType;
			write.dstBinding = binding;
			write.pBufferInfo = &info;
			write.descriptorCount = 1;
			write.dstSet = set;
			writes.emplace_back(write);
		}

		vkUpdateDescriptorSets(m_setLayout.m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}



	// DescriptorSet Builder -----------------------------------------------------

	DescriptorSet::Builder::Builder(VulkanDevice& device, DescriptorPool& pool, DescriptorSetLayout& setLayout)
		: m_device{ device }, m_pool{ pool }, m_setLayout{ setLayout }
	{
		m_writer.reserve(MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_writer.emplace_back(DescriptorWriter{ setLayout });
		}
	}

	DescriptorSet::Builder& DescriptorSet::Builder::addBuffer(uint32_t binding, const UniformBuffer& buffer)
	{
		for (size_t i = 0; i < m_writer.size(); i++)
		{
			m_writer[i].writeBuffer(binding, buffer.descriptorBufferInfo(i));
		}
		return *this;
	}

	DescriptorSet::Builder& DescriptorSet::Builder::addTexture(uint32_t binding, const Texture& texture)
	{
		for (size_t i = 0; i < m_writer.size(); i++)
		{
			m_writer[i].writeImage(binding, texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		return *this;
	}

	DescriptorSet::Builder& DescriptorSet::Builder::addTexture(uint32_t binding, std::shared_ptr<Texture> texture)
	{
		assert(texture != nullptr && "Cannot add Texture if it is nullptr");
		return addTexture(binding, *texture);
	}

	std::unique_ptr<DescriptorSet> DescriptorSet::Builder::build()
	{
		auto set = std::make_unique<DescriptorSet>(m_pool, m_setLayout);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_writer[i].build(set->m_descriptorSets[i]);
		}

		return set;
	}

	DescriptorSet::DescriptorSet(DescriptorPool& pool, DescriptorSetLayout& setLayout)
	{
		for (auto& set : m_descriptorSets)
		{
			auto result = pool.allocateDescriptorSet(setLayout.descriptorSetLayout(), set);
			assert(result && "Failed to allocate descriptor set");
		}
	}

	void DescriptorSet::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int index, VkPipelineBindPoint bindPoint) const
	{
		vkCmdBindDescriptorSets(commandBuffer, bindPoint, pipelineLayout, 0, 1, &m_descriptorSets[index], 0, nullptr);
	}
}
