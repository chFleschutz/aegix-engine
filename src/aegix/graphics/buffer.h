#pragma once

#include "graphics/device.h"

namespace Aegix::Graphics
{
	/// @brief Encapsulates a vulkan buffer
	class Buffer
	{
	public:
		Buffer(VulkanDevice& device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags bufferUsage,
			VmaAllocationCreateFlags allocFlags = 0, VkDeviceSize minOffsetAlignment = 1);
		Buffer(const Buffer&) = delete;
		~Buffer();

		Buffer& operator=(const Buffer&) = delete;
		
		operator VkBuffer() const { return m_buffer; }

		/// @brief Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
		void map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		/// @brief Unmap a mapped memory range
		void unmap();

		void write(const void* data);

		/// @brief Copies the specified data to the mapped buffer. Default value writes whole buffer range
		void write(const void* data, VkDeviceSize size, VkDeviceSize offset);

		/// @brief Flush a memory range of the buffer to make it visible to the device
		/// @note Only required for non-coherent memory. Also 'write' automatically flushes if needed
		void flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		/// @brief Create a buffer info descriptor
		/// @param size (Optional) Size of the memory range of the descriptor
		/// @param offset (Optional) Byte offset from beginning
		/// @return VkDescriptorBufferInfo of specified offset and range
		VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

		/// @brief Copies "instanceSize" bytes of data to the mapped buffer at an offset of index alignmentSize
		/// @param data Pointer to the data to copy
		/// @param index Used in offset calculation
		void writeToIndex(const void* data, int index);

		/// @brief Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
		/// @param index index Used in offset calculation
		/// @return VkResult of the flush call
		void flushIndex(int index);

		/// @brief Create a buffer info descriptor
		/// @param index Specifies the region given by index * alignmentSize
		/// @return VkDescriptorBufferInfo for instance at index
		VkDescriptorBufferInfo descriptorInfoForIndex(int index);

		VkBuffer buffer() const { return m_buffer; }
		void* mappedMemory() const { return m_mapped; }
		uint32_t instanceCount() const { return m_instanceCount; }
		VkDeviceSize instanceSize() const { return m_instanceSize; }
		VkDeviceSize alignmentSize() const { return m_instanceSize; }
		VkBufferUsageFlags usage() const { return m_usage; }
		VkMemoryPropertyFlags memoryProperties() const { return m_memoryProperties; }
		VkDeviceSize bufferSize() const { return m_bufferSize; }

	private:
		/// @brief Returns the minimum instance size required to be compatible with devices minOffsetAlignment
		/// @param instanceSize The size of an instance
		/// @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg minUniformBufferOffsetAlignment)
		/// @return VkResult of the buffer mapping call
		static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

		VulkanDevice& m_device;

		VkBuffer m_buffer = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		void* m_mapped = nullptr;

		VkDeviceSize m_bufferSize;
		uint32_t m_instanceCount;
		VkDeviceSize m_instanceSize;
		VkDeviceSize m_alignmentSize;
		VkBufferUsageFlags m_usage;
		VkMemoryPropertyFlags m_memoryProperties;
	};
}
