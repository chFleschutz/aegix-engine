#pragma once

#include "vre_device.h"

namespace vre
{
	/// @brief Encapsulates a vulkan buffer
	class VreBuffer
	{
	public:
		VreBuffer(
			VreDevice& device,
			VkDeviceSize instanceSize,
			uint32_t instanceCount,
			VkBufferUsageFlags usageFlags,
			VkMemoryPropertyFlags memoryPropertyFlags,
			VkDeviceSize minOffsetAlignment = 1);
		~VreBuffer();

		VreBuffer(const VreBuffer&) = delete;
		VreBuffer& operator=(const VreBuffer&) = delete;

		/// @brief Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
		/// @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
		/// @param offset (Optional) Byte offset from beginning
		/// @return VkResult of the buffer mapping call
		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		/// @brief Unmap a mapped memory range
		/// @note Does not return a result as vkUnmapMemory can't fail
		void unmap();

		/// @brief Copies the specified data to the mapped buffer. Default value writes whole buffer range
		/// @param data Pointer to the data to copy
		/// @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer range.
		/// @param offset (Optional) Byte offset from beginning of mapped region
		void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		/// @brief Flush a memory range of the buffer to make it visible to the device
		/// @note Only required for non-coherent memory
		/// @param size (Optional) Size of the memory range to flush (Pass VK_WHOLE_SIZE to flush the complete buffer range)
		/// @param offset (Optional) Byte offset from beginning
		/// @return VkResult of the flush call
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		/// @brief Invalidate a memory range of the buffer to make it visible to the host
		/// @note Only required for non-coherent memory
		/// @param size (Optional) Size of the memory range to invalidate (Pass VK_WHOLE_SIZE to invalidate the complete buffer range)
		/// @param offset (Optional) Byte offset from beginning
		/// @return VkResult of the invalidate call
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		/// @brief Create a buffer info descriptor
		/// @param size (Optional) Size of the memory range of the descriptor
		/// @param offset (Optional) Byte offset from beginning
		/// @return VkDescriptorBufferInfo of specified offset and range
		VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		/// @brief Copies "instanceSize" bytes of data to the mapped buffer at an offset of index alignmentSize
		/// @param data Pointer to the data to copy
		/// @param index Used in offset calculation
		void writeToIndex(void* data, int index);
		/// @brief Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
		/// @param index index Used in offset calculation
		/// @return VkResult of the flush call
		VkResult flushIndex(int index);
		/// @brief Create a buffer info descriptor
		/// @param index Specifies the region given by index * alignmentSize
		/// @return VkDescriptorBufferInfo for instance at index
		VkDescriptorBufferInfo descriptorInfoForIndex(int index);
		/// @brief Invalidate a memory range of the buffer to make it visible to the host
		/// @note Only required for non-coherent memory
		/// @param index Specifies the region to invalidate: index * alignmentSize
		/// @return VkResult of the invalidate call
		VkResult invalidateIndex(int index);

		VkBuffer buffer() const { return mBuffer; }
		void* mappedMemory() const { return mMapped; }
		uint32_t instanceCount() const { return mInstanceCount; }
		VkDeviceSize instanceSize() const { return mInstanceSize; }
		VkDeviceSize alignmentSize() const { return mInstanceSize; }
		VkBufferUsageFlags usageFlags() const { return mUsageFlags; }
		VkMemoryPropertyFlags memoryPropertyFlags() const { return mMemoryPropertyFlags; }
		VkDeviceSize bufferSize() const { return mBufferSize; }

	private:
		/// @brief Returns the minimum instance size required to be compatible with devices minOffsetAlignment
		/// @param instanceSize The size of an instance
		/// @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg minUniformBufferOffsetAlignment)
		/// @return VkResult of the buffer mapping call
		static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

		VreDevice& mVreDevice;
		void* mMapped = nullptr;
		VkBuffer mBuffer = VK_NULL_HANDLE;
		VkDeviceMemory mMemory = VK_NULL_HANDLE;

		VkDeviceSize mBufferSize;
		uint32_t mInstanceCount;
		VkDeviceSize mInstanceSize;
		VkDeviceSize mAlignmentSize;
		VkBufferUsageFlags mUsageFlags;
		VkMemoryPropertyFlags mMemoryPropertyFlags;
	};

}  // namespace lve