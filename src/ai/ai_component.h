#pragma once

#include "ai/option_manager.h"
#include "scripting/script_base.h"

namespace VEAI
{
    class AIComponent : public VEScripting::ScriptBase
    {
    public:
        AIComponent();
        ~AIComponent() = default;

        void update(float deltaSeconds) override;

    private:
        void handleInput();

        OptionManager m_optionManager;
    };

} // namespace VEAI