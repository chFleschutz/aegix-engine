#pragma once

#include "ai/reasoners/decision_tree.h"
#include "ai/considerations/consideration.h"
#include "ai/decision_tree_example/example_options.h"

namespace VEAI
{
	class MyDecisionTree
	{
	public:
		MyDecisionTree(VEAI::Blackboard& blackboard) 
		{
			blackboard.set<VEAI::BoolKnowledge>("PlayerNear", true);
			blackboard.set<VEAI::BoolKnowledge>("AtWar", false);
			blackboard.set<VEAI::FloatKnowledge>("Time", 0.6f);

			auto comp = new VEAI::AIComponent(blackboard);

			auto playerNearNode = m_tree.addRoot<VEAI::EntityDistanceConsideration>(blackboard, "Player", "NPC", 10.0f);

			auto atWarNode = VEAI::DecisionTree::addTrue<VEAI::BoolConsideration>(playerNearNode, blackboard, "AtWar");
			auto timeNode = VEAI::DecisionTree::addFalse<VEAI::ThresholdConsideration>(playerNearNode, blackboard, "Time", 0.5f);

			VEAI::DecisionTree::addTrue<VEAI::PatrolOption>(timeNode, comp);
			VEAI::DecisionTree::addFalse<VEAI::SleepOption>(timeNode, comp);

			VEAI::DecisionTree::addTrue<VEAI::MurderOption>(atWarNode, comp);
			VEAI::DecisionTree::addFalse<VEAI::GreetOption>(atWarNode, comp);
		}

		std::unique_ptr<Option> evaluate() const
		{
			return m_tree.evaluate();
		}

	private:
		DecisionTree m_tree;
	};
}