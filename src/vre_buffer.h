#pragma once

#include "vre_device.h"

namespace vre
{
	/*
	 * Encapsulates a vulkan buffer
	 */
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

		VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void unmap();

		void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		void writeToIndex(void* data, int index);
		VkResult flushIndex(int index);
		VkDescriptorBufferInfo descriptorInfoForIndex(int index);
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