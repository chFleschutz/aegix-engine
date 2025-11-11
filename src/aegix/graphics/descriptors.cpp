#include "pch.h"
#include "descriptors.h"

#include "graphics/resources/buffer.h"
#include "graphics/resources/texture.h"
#include "graphics/vulkan/vulkan_context.h"
#include "graphics/vulkan/vulkan_tools.h"

namespace Aegix::Graphics
{
	// DescriptorSetLayout Builder -----------------------------------------------------

	auto DescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType,
		VkShaderStageFlags stageFlags, uint32_t count) -> DescriptorSetLayout::Builder&
	{
		AGX_ASSERT_X(m_createInfo.bindings.count(binding) == 0, "Binding already in use");

		VkDescriptorSetLayoutBinding layoutBinding{
			.binding = binding,
			.descriptorType = descriptorType,
			.descriptorCount = count,
			.stageFlags = stageFlags,
		};
		m_createInfo.bindings[binding] = layoutBinding;
		return *this;
	}

	auto DescriptorSetLayout::Builder::setBindingFlags(VkDescriptorBindingFlags flags) -> Builder&
	{
		m_createInfo.bindingFlags = flags;
		return *this;
	}

	auto DescriptorSetLayout::Builder::setFlags(VkDescriptorSetLayoutCreateFlags flags) -> Builder&
	{
		m_createInfo.flags = flags;
		return *this;
	}

	auto DescriptorSetLayout::Builder::buildUnique() -> std::unique_ptr<DescriptorSetLayout>
	{
		return std::make_unique<DescriptorSetLayout>(m_createInfo);
	}

	auto DescriptorSetLayout::Builder::build() -> DescriptorSetLayout
	{
		return DescriptorSetLayout{ m_createInfo };
	}



	// DescriptorSetLayout -----------------------------------------------------

	DescriptorSetLayout::DescriptorSetLayout(CreateInfo& createInfo)
		: m_bindings{ std::move(createInfo.bindings) }
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
		setLayoutBindings.reserve(m_bindings.size());
		for (const auto& [location, binding] : m_bindings)
		{
			setLayoutBindings.emplace_back(binding);
		}

		std::vector<VkDescriptorBindingFlags> bindingFlagsVector(setLayoutBindings.size(), createInfo.bindingFlags);
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(bindingFlagsVector.size()),
			.pBindingFlags = bindingFlagsVector.data(),
		};

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = (createInfo.bindingFlags != 0) ? &bindingFlagsInfo : nullptr,
			.flags = createInfo.flags,
			.bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
			.pBindings = setLayoutBindings.data(),
		};

		VK_CHECK(vkCreateDescriptorSetLayout(VulkanContext::device(), &descriptorSetLayoutInfo, nullptr, &m_descriptorSetLayout))
	}

	DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
		: m_descriptorSetLayout{ other.m_descriptorSetLayout }, m_bindings{ std::move(other.m_bindings) }
	{
		other.m_descriptorSetLayout = VK_NULL_HANDLE;
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(VulkanContext::device(), m_descriptorSetLayout, nullptr);
	}

	auto DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept -> DescriptorSetLayout&
	{
		if (this != &other)
		{
			m_descriptorSetLayout = other.m_descriptorSetLayout;
			m_bindings = std::move(other.m_bindings);
			other.m_descriptorSetLayout = VK_NULL_HANDLE;
		}
		return *this;
	}

	auto DescriptorSetLayout::allocateDescriptorSet() const -> VkDescriptorSet
	{
		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
		VulkanContext::descriptorPool().allocateDescriptorSet(m_descriptorSetLayout, descriptorSet);
		return descriptorSet;
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

	DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
		: m_descriptorPool{ other.m_descriptorPool }
	{
		other.m_descriptorPool = VK_NULL_HANDLE;
	}

	DescriptorPool::~DescriptorPool()
	{
		if (m_descriptorPool)
		{
			vkDestroyDescriptorPool(VulkanContext::device(), m_descriptorPool, nullptr);
		}
	}

	auto DescriptorPool::operator=(DescriptorPool&& other) noexcept -> DescriptorPool&
	{
		if (this != &other)
		{
			m_descriptorPool = other.m_descriptorPool;
			other.m_descriptorPool = VK_NULL_HANDLE;
		}
		return *this;
	}

	void DescriptorPool::allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) const
	{
		VkDescriptorSetAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = m_descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &descriptorSetLayout,
		};

		VK_CHECK(vkAllocateDescriptorSets(VulkanContext::device(), &allocInfo, &descriptorSet))
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
		m_bufferInfos.emplace_back(binding, buffer.descriptorBufferInfo());
		return *this;
	}

	auto DescriptorWriter::writeBuffer(uint32_t binding, const Buffer& buffer, uint32_t index) -> DescriptorWriter&
	{
		m_bufferInfos.emplace_back(binding, buffer.descriptorBufferInfoFor(index));
		return *this;
	}

	auto DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo bufferInfo) -> DescriptorWriter&
	{
		m_bufferInfos.emplace_back(binding, std::move(bufferInfo));
		return *this;
	}

	void DescriptorWriter::update(VkDescriptorSet set)
	{
		std::vector<VkWriteDescriptorSet> writes;
		writes.reserve(m_bufferInfos.size() + m_imageInfos.size());

		for (const auto& [binding, info] : m_imageInfos)
		{
			AGX_ASSERT_X(m_setLayout.m_bindings.contains(binding), "Binding was not found in Descriptor Set Layout");
			auto& bindingDesc = m_setLayout.m_bindings[binding];
			AGX_ASSERT_X(bindingDesc.descriptorCount == 1, "Cannot write multiple images to a single descriptor");

			VkWriteDescriptorSet write{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = set,
				.dstBinding = binding,
				.dstArrayElement = 0, 
				.descriptorCount = 1,
				.descriptorType = bindingDesc.descriptorType,
				.pImageInfo = &info,
			};
			writes.emplace_back(write);
		}

		for (const auto& [binding, info] : m_bufferInfos)
		{
			AGX_ASSERT_X(m_setLayout.m_bindings.contains(binding), "Binding was not found in Descriptor Set Layout");
			auto& bindingDesc = m_setLayout.m_bindings[binding];
			AGX_ASSERT_X(bindingDesc.descriptorCount == 1, "Cannot write multiple buffers to a single descriptor");

			VkWriteDescriptorSet write{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = set,
				.dstBinding = binding,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = bindingDesc.descriptorType,
				.pBufferInfo = &info,
			};
			writes.emplace_back(write);
		}

		vkUpdateDescriptorSets(VulkanContext::device(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}



	// DescriptorSet Builder -----------------------------------------------------

	DescriptorSet::Builder::Builder(DescriptorSetLayout& setLayout)
		: m_setLayout{ setLayout }, m_writer{ setLayout, }
	{
	}

	auto DescriptorSet::Builder::addBuffer(uint32_t binding, const Buffer& buffer) -> DescriptorSet::Builder&
	{
		m_writer.writeBuffer(binding, buffer);
		return *this;
	}

	auto DescriptorSet::Builder::addTexture(uint32_t binding, const Texture& texture) -> DescriptorSet::Builder&
	{
		m_writer.writeImage(binding, texture);
		return *this;
	}

	auto DescriptorSet::Builder::addTexture(uint32_t binding, std::shared_ptr<Texture> texture) -> DescriptorSet::Builder&
	{
		AGX_ASSERT_X(texture != nullptr, "Cannot add Texture if it is nullptr");
		return addTexture(binding, *texture);
	}

	auto DescriptorSet::Builder::build() -> DescriptorSet
	{
		auto set = DescriptorSet{ m_setLayout };
		m_writer.update(set.descriptorSet());
		return set;
	}

	auto DescriptorSet::Builder::buildUnique() -> std::unique_ptr<DescriptorSet>
	{
		auto set = std::make_unique<DescriptorSet>(m_setLayout);
		m_writer.update(set->descriptorSet());
		return set;
	}



	// DescriptorSet -----------------------------------------------------

	DescriptorSet::DescriptorSet(DescriptorSetLayout& setLayout)
	{
		VulkanContext::descriptorPool().allocateDescriptorSet(setLayout, m_descriptorSet);
	}

	DescriptorSet::DescriptorSet(DescriptorSetLayout& setLayout, DescriptorPool& pool)
	{
		pool.allocateDescriptorSet(setLayout, m_descriptorSet);
	}

	void DescriptorSet::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout,
		VkPipelineBindPoint bindPoint) const
	{
		vkCmdBindDescriptorSets(commandBuffer, bindPoint, pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
	}
}
