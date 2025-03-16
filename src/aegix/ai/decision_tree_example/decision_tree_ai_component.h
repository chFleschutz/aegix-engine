#pragma once

#include "ai/ai_component.h"
#include "ai/decision_tree_example/my_decision_tree.h"

namespace Aegix::AI
{
    class DecisionTreeAiComponent : public AIComponent
    {
    public:
        DecisionTreeAiComponent(Blackboard& blackboard);

        void evalutate();
        void updateTime(float delta);
        void toggleWar();

        void update(float delta) override;

    private:
        MyDecisionTree m_decisionTree;
    };
} 
