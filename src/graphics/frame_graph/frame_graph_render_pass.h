#pragma once

#include "graphics/device.h"
#include "graphics/frame_info.h"
#include "graphics/texture.h"

namespace Aegix::Graphics
{
	struct FrameGraphResource
	{
		std::string name;
		std::shared_ptr<Texture> texture;
	};

	enum class ResourceUsage
	{
		ColorAttachment,
		DepthAttachment,
		SampledTexture,
		StorageTexture,
		UniformBuffer,
		StorageBuffer,
	};

	class FrameGraphPass
	{
	public:
		FrameGraphPass(VulkanDevice& device) : m_device{ device } {}
		FrameGraphPass(const FrameGraphPass&) = delete;
		virtual ~FrameGraphPass() = default;

		virtual void create() {};

		virtual void execute(const FrameInfo& frameInfo)
		{
			// TODO: Pass frameInfo to execute callback
			if (m_execute)
				m_execute();
		}

		void addInput(const std::shared_ptr<FrameGraphResource>& resource, ResourceUsage usage)
		{
			m_inputs.emplace_back(resource, usage);
		}

		void addOutput(const std::shared_ptr<FrameGraphResource>& resource, ResourceUsage usage)
		{
			m_outputs.emplace_back(resource, usage);
		}

		void setExecuteCallback(std::function<void()> execute) 
		{
			m_execute = execute;
		}

	protected:
		struct ResourceBinding
		{
			std::shared_ptr<FrameGraphResource> resource;
			ResourceUsage usage;
		};

		VulkanDevice& m_device;

		std::vector<ResourceBinding> m_inputs;
		std::vector<ResourceBinding> m_outputs;
		std::function<void()> m_execute;
	};
}