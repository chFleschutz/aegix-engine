#pragma once

#include "graphics/frame_info.h"
#include "graphics/texture.h"

namespace Aegix::Graphics
{
	struct Attachment
	{
		std::shared_ptr<Texture> texture;
	};

	class FrameGraphRenderPass
	{
	public:
		virtual ~FrameGraphRenderPass() = default;

		virtual void execute(const FrameInfo& frameInfo) = 0;

		void setAttachments(std::vector<Attachment> attachments) { m_attachments = std::move(attachments); }

	private:
		std::vector<Attachment> m_attachments;
	};
}