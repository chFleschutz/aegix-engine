#pragma once

#include "ai/knowledge.h"

namespace VEAI
{
    class AIComponent;

    class Option
    {
    public:
        Option(AIComponent* aiComponent) 
            : m_aiComponent(aiComponent) {}
        virtual ~Option() = default;    

        /// @brief Initializes the option with the AIComponent, duration and cooldown
        /// @note Call this function after creating the option and before using it
        void initialize(AIComponent* aiComponent, float duration = 0.0f, float cooldown = 0.0f);

        /// @brief Sets the duration of the option in seconds
        void setDuration(float seconds);
        /// @brief Sets the cooldown of the option in seconds
        void setCooldown(float seconds);

        /// @brief Returns true if the option is active otherwise false
        bool isActive() const { return m_isActive; }
        /// @brief Returns true if the option is on cooldown otherwise false
        bool onCooldown() const { return m_onCooldown; }
        /// @brief Returns the elapsed time since started
        float elapsedTime() const { return m_elapsedTime; }
        /// @brief Returns the overall duration
        float duration() const { return m_duration; }
        /// @brief Returns the base cooldown
        float cooldown() const { return m_cooldown; }

        /// @brief Start the option
        /// @note When overriding call base function first
        virtual void start();
        /// @brief Deactivates the option
        /// @note When overriding call base function first
        virtual void pause();
        /// @brief Deactivates the option and resets it
        /// @note When overriding call base function first
        virtual void stop();

        /// @brief Returns true if the option is finished otherwise false
        virtual bool isFinished() const { return m_finished; }

        /// @brief Updates the option
        void update(float deltaSeconds);

    protected:
        /// @brief Function is called once per frame only if the option is active
        /// @param deltaSeconds Seconds between frames
        /// @note Override this function to implement custom behaviour
        virtual void updateOption(float deltaSeconds) = 0;

        AIComponent* m_aiComponent = nullptr;

    private:
        bool m_finished = false;
        
        float m_duration = 0.0f;
        float m_elapsedTime = 0.0f;
        bool m_isActive = false;

        float m_cooldown = 0.0f;
        float m_cooldownTime = 0.0f;
        bool m_onCooldown = false;
    };
    
} // namespace VEAI