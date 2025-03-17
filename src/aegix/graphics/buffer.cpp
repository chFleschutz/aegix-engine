#include "pch.h"

#include "buffer.h"

#include "graphics/vulkan_tools.h"

namespace Aegix::Graphics
{
	VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
	{
		if (minOffsetAlignment > 0)
		{
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		}
		return instanceSize;
	}

	Buffer::Buffer(VulkanDevice& device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags bufferUsage,
		VmaAllocationCreateFlags allocFlags, VkDeviceSize minOffsetAlignment)
		: m_device{ device }, m_instanceSize{ instanceSize }, m_instanceCount{ instanceCount }, m_usage{ bufferUsage }
	{
		m_alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
		m_bufferSize = m_alignmentSize * instanceCount;

		device.createBuffer(m_buffer, m_allocation, m_bufferSize, bufferUsage, allocFlags, VMA_MEMORY_USAGE_AUTO);

		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(device.allocator(), m_allocation, &allocInfo);
		m_mapped = allocInfo.pMappedData;

		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device.physicalDevice(), &memProperties);
		m_memoryProperties = memProperties.memoryTypes[allocInfo.memoryType].propertyFlags;
	}

	Buffer::~Buffer()
	{
		unmap();

		m_device.destroyBuffer(m_buffer, m_allocation);
		m_buffer = VK_NULL_HANDLE;
		m_allocation = VK_NULL_HANDLE;
	}

	void Buffer::map(VkDeviceSize size, VkDeviceSize offset)
	{
		assert(m_buffer && m_allocation && "Called map on buffer before create");
		VK_CHECK(vmaMapMemory(m_device.allocator(), m_allocation, &m_mapped));
	}

	void Buffer::unmap()
	{
		if (m_mapped)
		{
			vmaUnmapMemory(m_device.allocator(), m_allocation);
			m_mapped = nullptr;
		}
	}

	void Buffer::write(const void* data)
	{
		write(data, m_bufferSize, 0);
	}

	void Buffer::write(const void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		VK_CHECK(vmaCopyMemoryToAllocation(m_device.allocator(), data, m_allocation, offset, size));
	}

	void Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VK_CHECK(vmaFlushAllocation(m_device.allocator(), m_allocation, offset, size));
	}

	VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) const
	{
		return VkDescriptorBufferInfo{ m_buffer, offset, size };
	}

	void Buffer::writeToIndex(const void* data, int index)
	{
		write(data, m_instanceSize, index * m_alignmentSize);
	}

	void Buffer::flushIndex(int index)
	{
		flush(m_alignmentSize, index * m_alignmentSize);
	}

	VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(int index)
	{
		return descriptorInfo(m_alignmentSize, index * m_alignmentSize);
	}
}
