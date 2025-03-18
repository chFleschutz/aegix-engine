#pragma once

#include "ai/reasoners/reasoner.h"

namespace Aegix::AI
{

	class DecisionTreeReasoner : public Reasoner
	{
	public:

	private:
		DecisionTree m_tree;
	};
}
