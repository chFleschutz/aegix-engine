#include "option.h"

namespace Aegis::AI
{
    void Option::initialize(AIComponent* aiComponent, float duration, float cooldown)
    {
        m_aiComponent = aiComponent;
		m_duration = duration;
		m_cooldown = cooldown;
    }

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
        m_finished = true;
        m_isActive = false;
        m_onCooldown = true;
        m_elapsedTime = 0.0f;
    }   

    void Option::update(float deltaSeconds)
    {
        // Update cooldown
        if (m_onCooldown)
        {
            if (m_cooldownTime > m_cooldown)
            {
                m_onCooldown = false;
                m_cooldownTime = 0.0f;
            }
            else
            {
                m_cooldownTime += deltaSeconds;
            }
        }

        // Update option 
        if (m_isActive)
        {
            m_elapsedTime += deltaSeconds;
            if (m_elapsedTime > m_duration and m_duration > 0.0f)
            {
                stop();
            }
            else
            {
                updateOption(deltaSeconds);
            }
        }
    }
}
