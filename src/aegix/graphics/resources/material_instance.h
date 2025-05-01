#pragma once

#include "graphics/resources/material_template.h"

namespace Aegix::Graphics
{
	class MaterialInstance
	{
	public:
		MaterialInstance(std::shared_ptr<MaterialTemplate> materialTemplate);
	
	private:
		std::shared_ptr<MaterialTemplate> m_template;
	};
}
