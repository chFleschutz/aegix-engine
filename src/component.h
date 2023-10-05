#pragma once

namespace vre
{
	class Component
	{
	public:
		/// @brief begin is called before the first Frame is rendered
		virtual void begin() {}

		/// @brief update is called once every frame
		/// @param deltaSeconds delay between the last two frames in seconds
		virtual void update(float deltaSeconds) {}

		/// @brief end is called after the last frame but before other ressources are cleaned up
		virtual void end() {}
	};

} // namespace vre
