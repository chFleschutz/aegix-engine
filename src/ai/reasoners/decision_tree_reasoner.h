#pragma once

#include "ai/reasoners/reasoner.h"

namespace VEAI
{

	class DecisionTreeReasoner : public Reasoner
	{
	public:

	private:
		DecisionTree m_tree;
	};

} // namespace VEAI
