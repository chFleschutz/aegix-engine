#pragma once

#include "ai/reasoners/reasoner.h"

namespace Aegis::AI
{

	class DecisionTreeReasoner : public Reasoner
	{
	public:

	private:
		DecisionTree m_tree;
	};
}
