#include "scutils/text/scstring.h"
#include "scutils/utils/scconfig.h"
#include "scutils/text/scstringlist.h"

#include "scutils/io/scloggerfactorybuilder.h"
#include "scutils/io/sclogger.h"

#include <string>
#include <iostream>

void sinkCallback(const ScStringView& msg)
{
	std::cout << std::string(msg.data(), msg.size()) << std::endl;
}

int main()
{
	do {
		ScLoggerFactory* pFactory = ScLoggerFactoryBuilder("Xde", "D:/Temp", "log.html")
			.setLoggerType(Sc::LoggerType::Console)
			.setLoggerLevel(Sc::LoggerLevel::Trace)
			//.setPattern("%^%Y-%m-%d %H:%M:%S %a [%8l]%$")
			.setLevelPattern({ { Sc::LoggerLevel::Info, "ÐÅÏ¢" },{ Sc::LoggerLevel::Warn, "¾¯¸æ" } })
			//.addSink(sinkCallback)
			.build();

		ScLoggerFactoryBuilder("Xde", "D:/Temp", "log.html")
			.setLoggerType(Sc::LoggerType::Console)
			.setLoggerLevel(Sc::LoggerLevel::Trace)
			.buildDefault();

		ScLoggerPtr logger = pFactory->logger("Test");
		SC_LOGGER_TRACE(logger, "this is {}", "test");
		SC_LOGGER_DEBUG(logger, "this is {}", "test");
		SC_LOGGER_INFO(logger, "this is {}", "test");
		SC_LOGGER_WARN(logger, "this is {}", "test");
		SC_LOGGER_ERROR(logger, "this is {}", "test");
		SC_LOGGER_CRITICAL(logger, "this is {}", "test");
		//ScLoggerPtr logger = pFactory->logger("Test");
		//logger->setLevel(Sc::LoggerLevel::Trace);
		//SC_LOGGER_INFO_IN(logger, "test =", 2, "test2 =", 2);
		//SC_LOGGER_INFO_OUT(logger);
		//SC_LOGGER_TRACE_IN(logger);
		//SC_LOGGER_TRACE(logger, "{}", "test");
		//SC_LOGGER_DEBUG(logger, "{}", "test");
		//SC_LOGGER_INFO(logger, "{}", "test");
		//SC_LOGGER_WARN(logger, "{}", "test");
		//SC_LOGGER_ERROR(logger, "{}", "test");
		//SC_LOGGER_CRITICAL(logger, "{}", "test");
	} while (0);
}