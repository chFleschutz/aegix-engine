#pragma once

#include "ai/option_manager.h"
#include "scripting/script_base.h"

namespace VEAI
{
    class AIComponent : public VEScripting::ScriptBase
    {
    public:
        AIComponent() = default;
        ~AIComponent() = default;

        void update(float deltaSeconds) override
        {
            m_optionManager.update(deltaSeconds);
        }

    protected:
        OptionManager m_optionManager;
    };

} // namespace VEAI