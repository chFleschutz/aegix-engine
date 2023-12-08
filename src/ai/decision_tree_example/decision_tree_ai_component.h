#pragma once

#include "ai/ai_component.h"
#include "ai/blackboard.h"
#include "ai/considerations/consideration.h"
#include "ai/options/option.h"
#include "ai/reasoners/decision_tree.h"
#include "scene/entity.h"
#include "scripting/script_base.h"

namespace VEAI
{
    class SleepOption : public Option
	{
    public:
        SleepOption(AIComponent* ai) : Option(ai) {}
        void start() override;
	};
    
    class PatrolOption : public Option
    {
    public:
        PatrolOption(AIComponent* ai) : Option(ai) {}
        void start() override;
    };
    
    class GreetOption : public Option
    {
    public:
        GreetOption(AIComponent* ai) : Option(ai) {}
        void start() override;
    };

    class MurderOption : public Option
	{
    public:
        MurderOption(AIComponent* ai) : Option(ai) {}
        void start() override;
	};

    class DecisionTreeAiComponent : public AIComponent
    {
    public:
        DecisionTreeAiComponent(Blackboard& blackboard);

        void evalutate();
        void toggleWar();

        void update(float delta) override;

    private:
        DecisionTree m_decisionTree;
    };

} // namespace VEAI