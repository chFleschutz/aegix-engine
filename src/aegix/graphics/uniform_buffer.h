#pragma once

#include "graphics/buffer.h"
#include "graphics/descriptors.h"
#include "graphics/globals.h"

namespace Aegix::Graphics
{
	/// @brief Manages uniform buffers of the gpu
	class UniformBuffer
	{
	public:
		template<typename T>
		UniformBuffer(VulkanDevice& device, const T& data)
		{
			for (auto& buffer : m_buffers)
			{
				buffer = std::make_unique<Buffer>(device, sizeof(T), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
					VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
				buffer->write(&data);
			}
		}

		UniformBuffer(VulkanDevice& device, const void* data, size_t size)
		{
			for (auto& buffer : m_buffers)
			{
				buffer = std::make_unique<Buffer>(device, size, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
				buffer->write(data);
			}
		}

		/// @brief Returns the descriptor info for the buffer
		VkDescriptorBufferInfo descriptorBufferInfo(int index) const { return m_buffers[index]->descriptorInfo(); }

		/// @brief Sets the data in the buffer for all frames
		/// @param data The new data
		/// @note This function should not be called once rendering has started (call setData(data, index) instead)
		template<typename T>
		void setData(const T& data, VkDeviceSize offset = 0)
		{
			for (auto& buffer : m_buffers)
			{
				buffer->write(&data, sizeof(T), offset);
			}
		}

		/// @brief Sets the data in the buffer for the specified frame index
		/// @param data The new data
		/// @param index The index of the currently active frame
		/// @note This function can be savely called during rendering
		template<typename T>
		void setData(int index, const T& data, VkDeviceSize offset = 0)
		{
			m_buffers[index]->write(&data, sizeof(T), offset);
		}

	private:
		std::array<std::unique_ptr<Buffer>, MAX_FRAMES_IN_FLIGHT> m_buffers;
	};


	/// @brief Manages gpu buffers for uniforms alongside the data
	/// @tparam T Type of data to be stored in the buffer
	/// @note If no cpu access to the data is needed, use UniformBuffer instead
	template<typename T>
	class UniformBufferData
	{
	public:
		UniformBufferData(VulkanDevice& device, const T& data = {})
			: m_data{ data }, m_buffer{ device, &m_data, sizeof(T) }
		{
		}

		VkDescriptorBufferInfo descriptorBufferInfo(int index) const { return m_buffer.descriptorBufferInfo(index); }

		const T& data() const { return m_data; }

		void setData(const T& data)
		{
			m_data = data;
			m_buffer.setData(data);
		}

		void setData(int index, const T& data)
		{
			m_data = data;
			m_buffer.setData(index, data);
		}

	private:
		T m_data;
		UniformBuffer m_buffer;
	};
}
