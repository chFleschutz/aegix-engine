#pragma once

#ifdef NDEBUG
#define AEGIS_LOG_DISABLE_DEBUG
#define AEGIS_LOG_DISABLE_TRACE
#endif // NDEBUG

#define AEGIS_LOG_FATAL_NO_ASYNC
#include <aegix-log/log.h>
#include <aegix-log/sinks/console_sink.h>

namespace Aegis
{
	class Logging
	{
	public:
		Logging()
		{
			ALOG::init<ALOG::DEFAULT_LOGGER, ALOG::NO_THREAD>(ALOG::Severity::Trace)
				.addSink<ALOG::ConsoleSink>();
		}

		static void logo()
		{
			ALOG::info("\n\n\n"
				"\t\t\t\t      ###  ########  ######   ##  ##     ##\n"
				"\t\t\t\t     ## ## ##       ##    ##  ##   ##   ## \n"
				"\t\t\t\t    ##  ## ##       ##        ##    ## ##  \n"
				"\t\t\t\t   ##   ## ######   ##        ##     ###   \n"
				"\t\t\t\t  ######## ##       ##  ####  ##    ## ##  \n"
				"\t\t\t\t ##     ## ##       ##    ##  ##   ##   ## \n"
				"\t\t\t\t##      ## ########  ######   ##  ##     ##\n"
				"\n\n");
		}
	};
}