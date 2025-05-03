#pragma once

#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	class MaterialTemplate
	{
	public:
		MaterialTemplate(Pipeline pipeline);

		template <typename T>
		void addParameter(const std::string& name, const T& value)
		{
			// TODO
		}

		void bind(VkCommandBuffer cmd);

	private:
		Pipeline m_pipeline;
	};
}
