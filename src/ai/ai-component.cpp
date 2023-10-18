#include "ai-component.h"

#include "ai/option.h"
#include "core/input.h"

namespace vai
{
    AIComponent::AIComponent()
    {
        m_option.setDuration(1.0f);
        m_option.setCooldown(0.2f);
    }

    void AIComponent::update(float deltaSeconds)
    {
        if (vre::Input::instance().keyPressed(GLFW_KEY_SPACE))
        {
            if (m_option.isActive())
            {
                m_option.pause();
            }
            else
            {
                m_option.start();
            }
        }

        m_option.update(deltaSeconds);
    }

} // namespace vai
