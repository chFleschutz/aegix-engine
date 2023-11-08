#pragma once

#include "scene/script_base.h"
#include "ai/options/option.h"

#include <memory>

namespace VEAI
{
    class AIComponent : public VEScripting::ScriptBase
    {
    public:
        AIComponent();
        ~AIComponent() = default;

        void update(float deltaSeconds) override;

    private:
        std::unique_ptr<Option> m_option;
    };

} // namespace VEAI