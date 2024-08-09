#pragma once

#include "ai/blackboard.h"
#include "ai/option_manager.h"
#include "scripting/script_base.h"

namespace Aegix::AI
{
    class AIComponent : public Aegix::Scripting::ScriptBase
    {
    public:
        AIComponent(Blackboard& blackboard) : m_blackboard(blackboard) {}

        void update(float deltaSeconds) override
        {
            m_optionManager.update(deltaSeconds);
        }

    protected:
        Blackboard& m_blackboard;

        OptionManager m_optionManager;
    };
}
