#pragma once

#include "scene/script_component_base.h"
#include "ai/option.h"

namespace vai
{
    class AIComponent : public vre::ScriptComponentBase
    {
    public:
        AIComponent();
        ~AIComponent() = default;

        void update(float deltaSeconds) override;

    private:
        Option m_option;
    };
}