#pragma once

namespace vre
{
	class Component
	{
	public:
		virtual void begin() {}
		virtual void update() {}
		virtual void end() {}
	};

} // namespace vre
