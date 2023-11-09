#pragma once

#include "utils/math_utils.h"

namespace VEAI
{
	struct Knowledge
	{
		Knowledge() = default;
		virtual ~Knowledge() = default;
	};


	struct PositionKnowledge : public Knowledge
	{
		Vector3 position;
	};

} // namespace VEAI