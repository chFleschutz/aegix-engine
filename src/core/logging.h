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