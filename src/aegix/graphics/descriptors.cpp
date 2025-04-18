#include "pch.h"

#include "descriptors.h"

#include "graphics/vulkan_tools.h"
#include "graphics/vulkan_context.h"

namespace Aegix::Graphics
{
	// DescriptorSetLayout Builder -----------------------------------------------------

	auto DescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType,
		VkShaderStageFlags stageFlags, uint32_t count) -> DescriptorSetLayout::Builder&
	{
		AGX_ASSERT_X(m_bindings.count(binding) == 0, "Binding already in use");

		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;
		m_bindings[binding] = layoutBinding;

		return *this;
	}

	auto DescriptorSetLayout::Builder::build() const -> std::unique_ptr<DescriptorSetLayout>
	{
		return std::make_unique<DescriptorSetLayout>(m_bindings);
	}



	// DescriptorSetLayout -----------------------------------------------------

	DescriptorSetLayout::DescriptorSetLayout(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
		: m_bindings{ bindings }
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

		VK_CHECK(vkCreateDescriptorSetLayout(VulkanContext::device(), &descriptorSetLayoutInfo, nullptr, &m_descriptorSetLayout))
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(VulkanContext::device(), m_descriptorSetLayout, nullptr);
	}



	// DescriptorPool Builder -----------------------------------------------------

	DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
	{
		m_poolSizes.push_back({ descriptorType, count });
		return *this;
	}

	auto DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) -> DescriptorPool::Builder&
	{
		m_poolFlags = flags;
		return *this;
	}

	auto DescriptorPool::Builder::setMaxSets(uint32_t count) -> DescriptorPool::Builder&
	{
		m_maxSets = count;
		return *this;
	}

	auto DescriptorPool::Builder::build() const -> DescriptorPool
	{
		return DescriptorPool{ m_maxSets, m_poolFlags, m_poolSizes };
	}

	auto DescriptorPool::Builder::buildUnique() const -> std::unique_ptr<DescriptorPool>
	{
		return std::make_unique<DescriptorPool>(m_maxSets, m_poolFlags, m_poolSizes);
	}



	// DescriptorPool -----------------------------------------------------

	DescriptorPool::DescriptorPool(uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
		const std::vector<VkDescriptorPoolSize>& poolSizes)
	{
		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = poolFlags;

		VK_CHECK(vkCreateDescriptorPool(VulkanContext::device(), &descriptorPoolInfo, nullptr, &m_descriptorPool))
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(VulkanContext::device(), m_descriptorPool, nullptr);
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
		if (vkAllocateDescriptorSets(VulkanContext::device(), &allocInfo, &descriptor) != VK_SUCCESS)
			return false;

		return true;
	}

	void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
	{
		vkFreeDescriptorSets(VulkanContext::device(), m_descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
	}

	void DescriptorPool::resetPool()
	{
		vkResetDescriptorPool(VulkanContext::device(), m_descriptorPool, 0);
	}



	// DescriptorWriter -----------------------------------------------------

	DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout)
		: m_setLayout{ setLayout }
	{
	}

	auto DescriptorWriter::writeImage(uint32_t binding, const Texture& texture) -> DescriptorWriter&
	{
		m_imageInfos.emplace_back(binding, texture.descriptorImageInfo());
		return *this;
	}

	auto DescriptorWriter::writeImage(uint32_t binding, const Texture& texture, VkImageLayout layoutOverride) -> DescriptorWriter&
	{
		m_imageInfos.emplace_back(binding, texture.descriptorImageInfo(layoutOverride));
		return *this;
	}

	auto DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo imageInfo) -> DescriptorWriter&
	{
		m_imageInfos.emplace_back(binding, std::move(imageInfo));
		return *this;
	}

	auto DescriptorWriter::writeBuffer(uint32_t binding, const Buffer& buffer) -> DescriptorWriter&
	{
		m_bufferInfos.emplace_back(binding, buffer.descriptorInfo());
		return *this;
	}

	auto DescriptorWriter::writeBuffer(uint32_t binding, const Buffer& buffer, uint32_t index) -> DescriptorWriter&
	{
		m_bufferInfos.emplace_back(binding, buffer.descriptorInfoForIndex(index));
		return *this;
	}

	auto DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo bufferInfo) -> DescriptorWriter&
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
			AGX_ASSERT_X(bindingDesc.descriptorCount == 1, "Cannot write multiple images to a single descriptor");

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
			AGX_ASSERT_X(bindingDesc.descriptorCount == 1, "Cannot write multiple images to a single descriptor");

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorType = bindingDesc.descriptorType;
			write.dstBinding = binding;
			write.pBufferInfo = &info;
			write.descriptorCount = 1;
			write.dstSet = set;
			writes.emplace_back(write);
		}

		vkUpdateDescriptorSets(VulkanContext::device(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}



	// DescriptorSet Builder -----------------------------------------------------

	DescriptorSet::Builder::Builder(DescriptorPool& pool, DescriptorSetLayout& setLayout)
		: m_pool{ pool }, m_setLayout{ setLayout }
	{
		m_writer.reserve(MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_writer.emplace_back(DescriptorWriter{ setLayout });
		}
	}

	auto DescriptorSet::Builder::addBuffer(uint32_t binding, const Buffer& buffer) -> DescriptorSet::Builder&
	{
		for (size_t i = 0; i < m_writer.size(); i++)
		{
			m_writer[i].writeBuffer(binding, buffer.descriptorInfoForIndex(i));
		}
		return *this;
	}

	auto DescriptorSet::Builder::addTexture(uint32_t binding, const Texture& texture) -> DescriptorSet::Builder&
	{
		for (size_t i = 0; i < m_writer.size(); i++)
		{
			m_writer[i].writeImage(binding, texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		return *this;
	}

	auto DescriptorSet::Builder::addTexture(uint32_t binding, std::shared_ptr<Texture> texture) -> DescriptorSet::Builder&
	{
		AGX_ASSERT_X(texture != nullptr, "Cannot add Texture if it is nullptr");
		return addTexture(binding, *texture);
	}

	auto DescriptorSet::Builder::build() -> std::unique_ptr<DescriptorSet>
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
			AGX_ASSERT_X(result, "Failed to allocate descriptor set");
		}
	}

	void DescriptorSet::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int index, VkPipelineBindPoint bindPoint) const
	{
		vkCmdBindDescriptorSets(commandBuffer, bindPoint, pipelineLayout, 0, 1, &m_descriptorSets[index], 0, nullptr);
	}
}
