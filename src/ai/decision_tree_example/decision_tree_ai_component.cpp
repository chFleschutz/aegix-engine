#include "decision_tree_ai_component.h"

#include "core/input.h"

namespace VEAI
{
    void SleepOption::start()
    {
        Option::start();
        std::cout << "Sleeping... zZz ZzZ zZz" << std::endl;
    }

    void PatrolOption::start()
    {
        Option::start();
        std::cout << "Patrolling..." << std::endl;
    }

    void GreetOption::start()
    {
        Option::start();
        std::cout << "Greetings my friend" << std::endl;
    }

    void MurderOption::start()
    {
        Option::start();
        std::cout << "Murdering... Muhahahaaa" << std::endl;
    }

    DecisionTreeAiComponent::DecisionTreeAiComponent(Blackboard& blackboard)
        : AIComponent(blackboard)
    {
        auto& input = Input::instance();
        input.bind(this, &DecisionTreeAiComponent::toggleWar, Input::One);

        auto playerNearNode = m_decisionTree.addRoot<EntityDistanceConsideration>(m_blackboard, "Player", "NPC", 10.0f);

        auto timeNode = playerNearNode->addFalse<ThresholdConsideration>(m_blackboard, "Time", 0.5f);
        auto atWarNode = playerNearNode->addTrue<BoolConsideration>(m_blackboard, "AtWar");

        timeNode->addFalse<SleepOption>(this);
        timeNode->addTrue<PatrolOption>(this);

        atWarNode->addFalse<GreetOption>(this);
        atWarNode->addTrue<MurderOption>(this);
    }

    void DecisionTreeAiComponent::evalutate()
    {
        auto option = m_decisionTree.evaluate();

        if (option == nullptr)
            return;

        option->start();
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
