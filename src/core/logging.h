#pragma once

#include <aegix-log/log.h>
#include <aegix-log/sinks/console_sink.h>

namespace Aegix
{
	class Logging
	{
	public:
		Logging()
		{
			ALOG::init(ALOG::Severity::Trace)
				.addSink<ALOG::ConsoleSink>();
		}
	};
}