#include "scutils/text/scstring.h"
#include "scutils/utils/scconfig.h"
#include "scutils/text/scstringlist.h"

#include "scutils/io/scloggerfactorybuilder.h"
#include "scutils/io/sclogger.h"

#include <string>
#include <iostream>

int main()
{
	do {
		ScLoggerFactory* pFactory = ScLoggerFactoryBuilder("Xde", "D:/Temp", "log.html")
			.setLoggerType(Sc::LoggerType::Console)
			.build();
;
		ScLoggerPtr logger = pFactory->logger("Test");
		logger->setLevel(Sc::LoggerLevel::Trace);
		SC_LOGGER_INFO_IN(logger, "test =", 2, "test2 =", 2);
		SC_LOGGER_INFO_OUT(logger);
		//SC_LOGGER_TRACE_IN(logger);
		//SC_LOGGER_TRACE(logger, "{}", "test");
		//SC_LOGGER_DEBUG(logger, "{}", "test");
		//SC_LOGGER_INFO(logger, "{}", "test");
		//SC_LOGGER_WARN(logger, "{}", "test");
		//SC_LOGGER_ERROR(logger, "{}", "test");
		//SC_LOGGER_CRITICAL(logger, "{}", "test");
	} while (1);
}