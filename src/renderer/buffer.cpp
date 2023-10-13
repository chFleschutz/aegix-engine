#include "buffer.h"

#include <cassert>
#include <cstring>

namespace vre
{
    VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
    {
        if (minOffsetAlignment > 0)
        {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    Buffer::Buffer(
        VulkanDevice& device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment)
        : 
        mDevice{ device },
        mInstanceSize{ instanceSize },
        mInstanceCount{ instanceCount },
        mUsageFlags{ usageFlags },
        mMemoryPropertyFlags{ memoryPropertyFlags }
    {
        mAlignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        mBufferSize = mAlignmentSize * instanceCount;
        device.createBuffer(mBufferSize, usageFlags, memoryPropertyFlags, mBuffer, mMemory);
    }

    Buffer::~Buffer()
    {
        unmap();
        vkDestroyBuffer(mDevice.device(), mBuffer, nullptr);
        vkFreeMemory(mDevice.device(), mMemory, nullptr);
    }

    VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset)
    {
        assert(mBuffer && mMemory && "Called map on buffer before create");
        return vkMapMemory(mDevice.device(), mMemory, offset, size, 0, &mMapped);
    }

    void Buffer::unmap()
    {
        if (mMapped)
        {
            vkUnmapMemory(mDevice.device(), mMemory);
            mMapped = nullptr;
        }
    }

    void Buffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset)
    {
        assert(mMapped && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE)
        {
            memcpy(mMapped, data, mBufferSize);
        }
        else
        {
            char* memOffset = (char*)mMapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = mMemory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(mDevice.device(), 1, &mappedRange);
    }

    VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = mMemory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(mDevice.device(), 1, &mappedRange);
    }

    VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset)
    {
        return VkDescriptorBufferInfo{
            mBuffer,
            offset,
            size,
        };
    }

    void Buffer::writeToIndex(void* data, int index)
    {
        writeToBuffer(data, mInstanceSize, index * mAlignmentSize);
    }

    VkResult Buffer::flushIndex(int index) 
    { 
        return flush(mAlignmentSize, index * mAlignmentSize); 
    }

    VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(int index)
    {
        return descriptorInfo(mAlignmentSize, index * mAlignmentSize);
    }

    VkResult Buffer::invalidateIndex(int index)
    {
        return invalidate(mAlignmentSize, index * mAlignmentSize);
    }

}  // namespace vre