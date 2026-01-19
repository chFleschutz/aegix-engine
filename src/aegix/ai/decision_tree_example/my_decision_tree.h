#pragma once

#include "ai/reasoners/decision_tree.h"
#include "ai/considerations/consideration.h"
#include "ai/decision_tree_example/example_options.h"

namespace Aegis::AI
{
	class MyDecisionTree
	{
	public:
		MyDecisionTree(Aegis::AI::Blackboard& blackboard) 
		{
			blackboard.set<Aegis::AI::BoolKnowledge>("PlayerNear", true);
			blackboard.set<Aegis::AI::BoolKnowledge>("AtWar", false);
			blackboard.set<Aegis::AI::FloatKnowledge>("Time", 0.6f);

			auto comp = new Aegis::AI::AIComponent(blackboard);

			auto playerNearNode = m_tree.addRoot<Aegis::AI::EntityDistanceConsideration>(blackboard, "Player", "NPC", 10.0f);

			auto atWarNode = Aegis::AI::DecisionTree::addTrue<Aegis::AI::BoolConsideration>(playerNearNode, blackboard, "AtWar");
			auto timeNode = Aegis::AI::DecisionTree::addFalse<Aegis::AI::ThresholdConsideration>(playerNearNode, blackboard, "Time", 0.5f);

			Aegis::AI::DecisionTree::addTrue<Aegis::AI::PatrolOption>(timeNode, comp);
			Aegis::AI::DecisionTree::addFalse<Aegis::AI::SleepOption>(timeNode, comp);

			Aegis::AI::DecisionTree::addTrue<Aegis::AI::MurderOption>(atWarNode, comp);
			Aegis::AI::DecisionTree::addFalse<Aegis::AI::GreetOption>(atWarNode, comp);
		}

		std::unique_ptr<Option> evaluate() const
		{
			return m_tree.evaluate();
		}

	private:
		DecisionTree m_tree;
	};
}