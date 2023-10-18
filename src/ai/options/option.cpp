#include "option.h"

namespace vai
{
    void Option::setDuration(float seconds)
    {
        m_duration = seconds;
    }

    void Option::setCooldown(float seconds)
    {
        m_cooldown = seconds;
    }

    void Option::start()
    {
        if (m_onCooldown)
            return;

        m_isActive = true;
    }

    void Option::pause()
    {
        m_isActive = false;
        m_onCooldown = true;
    }

    void Option::stop()
    {
        m_isActive = false;
        m_elapsedTime = 0.0f;
    }   

    void Option::update(float deltaSeconds)
    {
        // Update cooldown
        if (m_onCooldown)
        {
            m_cooldownTime += deltaSeconds;
            if (m_cooldownTime > m_cooldown)
            {
                m_onCooldown = false;
                m_cooldownTime = 0.0f;
            }
        }

        // Update option 
        if (m_isActive)
        {
            m_elapsedTime += deltaSeconds;
            updateOption(deltaSeconds);

            if (m_elapsedTime > m_duration)
                stop();
        }
    }

} // namespace vai