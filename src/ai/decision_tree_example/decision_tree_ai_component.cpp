#include "decision_tree_ai_component.h"

#include "ai/decision_tree_example/example_options.h"
#include "ai/considerations/consideration.h"
#include "core/input.h"


namespace VEAI
{
    DecisionTreeAiComponent::DecisionTreeAiComponent(Blackboard& blackboard)
        : AIComponent(blackboard), m_decisionTree(blackboard)
    {
        auto& input = Input::instance();
        input.bind(this, &DecisionTreeAiComponent::toggleWar, Input::One);
    }

    void DecisionTreeAiComponent::evalutate()
    {
        auto option = m_decisionTree.evaluate();

        if (option == nullptr)
            return;

        m_optionManager.emplacePrioritized(std::move(option));
    }

    void DecisionTreeAiComponent::updateTime(float delta)
    {
        auto time = m_blackboard.get<FloatKnowledge>("Time")->value;
        time += delta / 3.0f;

        if (time > 1.0f)
            time = 0.0f;

        m_blackboard.set<FloatKnowledge>("Time", time);
    }

    void DecisionTreeAiComponent::toggleWar()
    {
        auto atWar = m_blackboard.get<BoolKnowledge>("AtWar")->value;
        m_blackboard.set<BoolKnowledge>("AtWar", !atWar);

        std::cout << "AtWar: " << m_blackboard.get<BoolKnowledge>("AtWar")->value << std::endl;
    }

    void DecisionTreeAiComponent::update(float delta)
    {
        updateTime(delta);
        evalutate();
    }

} // namespace VEAI
