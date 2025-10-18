#include "pch.h"
#include "buffer.h"

#include "graphics/vulkan/vulkan_context.h"
#include "graphics/vulkan/vulkan_tools.h"

namespace Aegix::Graphics
{
	auto Buffer::createUniformBuffer(VkDeviceSize size, uint32_t instanceCount) -> Buffer
	{
		auto aligment = VulkanContext::device().properties().limits.minUniformBufferOffsetAlignment;
		return Buffer{ size, instanceCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, aligment };
	}

	auto Buffer::createStorageBuffer(VkDeviceSize size, uint32_t instanceCount) -> Buffer
	{
		auto aligment = VulkanContext::device().properties().limits.minStorageBufferOffsetAlignment;
		return Buffer{ size, instanceCount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, aligment };
	}

	auto Buffer::createVertexBuffer(VkDeviceSize size, uint32_t instanceCount) -> Buffer
	{
		return Buffer{ size, instanceCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT };
	}

	auto Buffer::createIndexBuffer(VkDeviceSize size, uint32_t instanceCount) -> Buffer
	{
		return Buffer{ size, instanceCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT };
	}

	auto Buffer::createStagingBuffer(VkDeviceSize size) -> Buffer
	{
		return Buffer{ size, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT };
	}



	Buffer::Buffer(VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags bufferUsage,
		VmaAllocationCreateFlags allocFlags, VkDeviceSize minOffsetAlignment)
		: m_instanceSize{ instanceSize }, m_instanceCount{ instanceCount }, m_usage{ bufferUsage }
	{
		m_alignmentSize = computeAlignment(instanceSize, minOffsetAlignment);
		m_bufferSize = m_alignmentSize * instanceCount;

		VulkanContext::device().createBuffer(m_buffer, m_allocation, m_bufferSize, bufferUsage, allocFlags, VMA_MEMORY_USAGE_AUTO);

		VmaAllocationInfo allocInfo;
		vmaGetAllocationInfo(VulkanContext::device().allocator(), m_allocation, &allocInfo);
		m_mapped = allocInfo.pMappedData;
	}

	Buffer::Buffer(Buffer&& other) noexcept
		: m_buffer{ other.m_buffer }, m_allocation{ other.m_allocation },
		m_bufferSize{ other.m_bufferSize }, m_instanceSize{ other.m_instanceSize }, m_alignmentSize{ other.m_alignmentSize },
		m_instanceCount{ other.m_instanceCount }, m_usage{ other.m_usage }, m_mapped{ other.m_mapped }
	{
		other.m_buffer = VK_NULL_HANDLE;
		other.m_allocation = VK_NULL_HANDLE;
		other.m_bufferSize = 0;
		other.m_instanceSize = 0;
		other.m_alignmentSize = 0;
		other.m_instanceCount = 0;
		other.m_usage = 0;
		other.m_mapped = nullptr;
	}

	Buffer::~Buffer()
	{
		destroy();
	}

	auto Buffer::operator=(Buffer&& other) noexcept -> Buffer&
	{
		if (this != &other)
			return *this;

		destroy();

		m_buffer = other.m_buffer;
		m_allocation = other.m_allocation;
		m_bufferSize = other.m_bufferSize;
		m_instanceSize = other.m_instanceSize;
		m_alignmentSize = other.m_alignmentSize;
		m_instanceCount = other.m_instanceCount;
		m_usage = other.m_usage;
		m_mapped = other.m_mapped;

		other.m_buffer = VK_NULL_HANDLE;
		other.m_allocation = VK_NULL_HANDLE;
		other.m_bufferSize = 0;
		other.m_instanceSize = 0;
		other.m_alignmentSize = 0;
		other.m_instanceCount = 0;
		other.m_usage = 0;
		other.m_mapped = nullptr;

		return *this;
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
		VK_CHECK(vmaMapMemory(VulkanContext::device().allocator(), m_allocation, &m_mapped));
	}

	void Buffer::unmap()
	{
		if (m_mapped)
		{
			vmaUnmapMemory(VulkanContext::device().allocator(), m_allocation);
			m_mapped = nullptr;
		}
	}

	void Buffer::write(const void* data)
	{
		write(data, m_bufferSize, 0);
	}

	void Buffer::write(const void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		AGX_ASSERT_X(m_mapped, "Called write on buffer before map");

		memcpy(static_cast<uint8_t*>(m_mapped) + offset, data, size);
		flush(size, offset);
	}

	void Buffer::writeToIndex(const void* data, int index)
	{
		AGX_ASSERT_X(m_mapped, "Called write on buffer before map");

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
		VK_CHECK(vmaCopyMemoryToAllocation(VulkanContext::device().allocator(), data, m_allocation, offset, size));
	}

	void Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VK_CHECK(vmaFlushAllocation(VulkanContext::device().allocator(), m_allocation, offset, size));
	}

	void Buffer::flushIndex(int index)
	{
		flush(m_alignmentSize, index * m_alignmentSize);
	}

	void Buffer::upload(const void* data, VkDeviceSize size)
	{
		Buffer stagingBuffer = Buffer::createStagingBuffer(size);
		stagingBuffer.singleWrite(data, size, 0);
		stagingBuffer.copyTo(*this, size);
	}

	void Buffer::copyTo(Buffer& dest, VkDeviceSize size)
	{
		VulkanContext::device().copyBuffer(m_buffer, dest.m_buffer, size);
	}

	auto Buffer::computeAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) -> VkDeviceSize
	{
		if (minOffsetAlignment > 0)
		{
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		}

		return instanceSize;
	}

	void Buffer::destroy()
	{
		VulkanContext::destroy(m_buffer, m_allocation);
		m_buffer = VK_NULL_HANDLE;
		m_allocation = VK_NULL_HANDLE;
	}
}
