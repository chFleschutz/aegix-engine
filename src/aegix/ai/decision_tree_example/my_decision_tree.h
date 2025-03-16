#pragma once

#include "ai/reasoners/decision_tree.h"
#include "ai/considerations/consideration.h"
#include "ai/decision_tree_example/example_options.h"

namespace Aegix::AI
{
	class MyDecisionTree
	{
	public:
		MyDecisionTree(Aegix::AI::Blackboard& blackboard) 
		{
			blackboard.set<Aegix::AI::BoolKnowledge>("PlayerNear", true);
			blackboard.set<Aegix::AI::BoolKnowledge>("AtWar", false);
			blackboard.set<Aegix::AI::FloatKnowledge>("Time", 0.6f);

			auto comp = new Aegix::AI::AIComponent(blackboard);

			auto playerNearNode = m_tree.addRoot<Aegix::AI::EntityDistanceConsideration>(blackboard, "Player", "NPC", 10.0f);

			auto atWarNode = Aegix::AI::DecisionTree::addTrue<Aegix::AI::BoolConsideration>(playerNearNode, blackboard, "AtWar");
			auto timeNode = Aegix::AI::DecisionTree::addFalse<Aegix::AI::ThresholdConsideration>(playerNearNode, blackboard, "Time", 0.5f);

			Aegix::AI::DecisionTree::addTrue<Aegix::AI::PatrolOption>(timeNode, comp);
			Aegix::AI::DecisionTree::addFalse<Aegix::AI::SleepOption>(timeNode, comp);

			Aegix::AI::DecisionTree::addTrue<Aegix::AI::MurderOption>(atWarNode, comp);
			Aegix::AI::DecisionTree::addFalse<Aegix::AI::GreetOption>(atWarNode, comp);
		}

		std::unique_ptr<Option> evaluate() const
		{
			return m_tree.evaluate();
		}

	private:
		DecisionTree m_tree;
	};
}