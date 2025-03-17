#include "pch.h"

#include "buffer.h"

#include "graphics/vulkan_tools.h"

namespace Aegix::Graphics
{
	auto Buffer::createUniformBuffer(VulkanDevice& device, VkDeviceSize size, uint32_t instanceCount) -> Buffer
	{
		auto aligment = device.properties().limits.minUniformBufferOffsetAlignment;
		return Buffer{ device, size, instanceCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, aligment };
	}

	auto Buffer::createStagingBuffer(VulkanDevice& device, VkDeviceSize size) -> Buffer
	{
		return Buffer{ device, size, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT };
	}

	Buffer::Buffer(VulkanDevice& device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags bufferUsage,
		VmaAllocationCreateFlags allocFlags, VkDeviceSize minOffsetAlignment)
		: m_device{ device }, m_instanceSize{ instanceSize }, m_instanceCount{ instanceCount }, m_usage{ bufferUsage }
	{
		m_alignmentSize = computeAlignment(instanceSize, minOffsetAlignment);
		m_bufferSize = m_alignmentSize * instanceCount;

		device.createBuffer(m_buffer, m_allocation, m_bufferSize, bufferUsage, allocFlags, VMA_MEMORY_USAGE_AUTO);

		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(device.allocator(), m_allocation, &allocInfo);
		m_mapped = allocInfo.pMappedData;
	}

	Buffer::~Buffer()
	{
		m_device.destroyBuffer(m_buffer, m_allocation);
		m_buffer = VK_NULL_HANDLE;
		m_allocation = VK_NULL_HANDLE;
	}

	auto Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) const -> VkDescriptorBufferInfo
	{
		return VkDescriptorBufferInfo{ m_buffer, offset, size };
	}

	auto Buffer::descriptorInfoForIndex(int index) const -> VkDescriptorBufferInfo
	{
		return descriptorInfo(m_alignmentSize, index * m_alignmentSize);
	}

	void Buffer::map()
	{
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
		assert(m_mapped && "Called write on buffer before map");

		memcpy(static_cast<uint8_t*>(m_mapped) + offset, data, size);
		flush(size, offset);
	}

	void Buffer::writeToIndex(const void* data, int index)
	{
		assert(m_mapped && "Called write on buffer before map");

		memcpy(static_cast<uint8_t*>(m_mapped) + (index * m_alignmentSize), data, m_instanceSize);
		flushIndex(index);
	}

	void Buffer::singleWrite(const void* data)
	{
		if (m_instanceCount == 1)
		{
			singleWrite(data, m_instanceSize, 0);
		}
		else // Copy data to all instances
		{
			map();
			for (uint32_t i = 0; i < m_instanceCount; i++)
			{
				memcpy(static_cast<uint8_t*>(m_mapped) + (i * m_alignmentSize), data, m_instanceSize);
			}
			flush(m_bufferSize, 0);
			unmap();
		}
	}

	void Buffer::singleWrite(const void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		VK_CHECK(vmaCopyMemoryToAllocation(m_device.allocator(), data, m_allocation, offset, size));
	}

	void Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VK_CHECK(vmaFlushAllocation(m_device.allocator(), m_allocation, offset, size));
	}

	void Buffer::flushIndex(int index)
	{
		flush(m_alignmentSize, index * m_alignmentSize);
	}

	void Buffer::upload(const void* data, VkDeviceSize size)
	{
		Buffer stagingBuffer = Buffer::createStagingBuffer(m_device, size);
		stagingBuffer.singleWrite(data, size, 0);
		stagingBuffer.copyTo(*this, size);
	}

	void Buffer::copyTo(Buffer& dest, VkDeviceSize size)
	{
		m_device.copyBuffer(m_buffer, dest.m_buffer, size);
	}

	auto Buffer::computeAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) -> VkDeviceSize
	{
		if (minOffsetAlignment > 0)
		{
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		}

		return instanceSize;
	}
}
