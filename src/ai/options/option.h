#pragma once

namespace vai
{
    class Option
    {
    public:
        Option() = default;
        virtual ~Option() = default;    

        /// @brief Sets the duration of the option in seconds
        void setDuration(float seconds);
        /// @brief Sets the cooldown of the option in seconds
        void setCooldown(float seconds);

        /// @brief Returns true if the option is active otherwise false
        bool isActive() const { return m_isActive; }
        /// @brief Returns true if the option is on cooldown otherwise false
        bool onCooldown() const { return m_onCooldown; }
        /// @brief Returns the elapsed time 
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

        /// @brief Updates the option
        void update(float deltaSeconds);
        
        /// @brief Function is called once per frame if the option is active
        /// @param deltaSeconds Seconds between frames
        /// @note Override this function to implement custom behaviour
        virtual void updateOption(float deltaSeconds) = 0;

    private:
        float m_duration = 0.0f;
        float m_elapsedTime = 0.0f;
        bool m_isActive = false;

        float m_cooldown = 0.0f;
        float m_cooldownTime = 0.0f;
        bool m_onCooldown = false;
    };
    
} // namespace vai