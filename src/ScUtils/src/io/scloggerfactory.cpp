#include "scutils/io/scloggerfactory.h"

#include "scutils/io/scloggerfactorybuilder.h"

#include "scutils/io/sclogger_p.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/hourly_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/async.h"

ScLoggerFactory::ScLoggerFactory(ScLoggerFactoryData* d)
	: d(d)
{
}

ScLoggerFactory::~ScLoggerFactory()
{
	SC_SAVE_DELETE(d);
}

ScLoggerPtr ScLoggerFactory::logger(const ScString& loggerName)
{
	auto it = d->g_loggerMap.find(loggerName);
	if (d->g_loggerMap.end() == it)
	{
		std::lock_guard<std::mutex> locker(d->g_loggerMapMutex);
		it = d->g_loggerMap.find(loggerName);
		if (d->g_loggerMap.end() == it)
		{
			ScLoggerPtr logger = d->createLogger(loggerName);
			d->g_loggerMap[loggerName] = logger;
			return logger;
		}
	}
	return it->second;
}

ScLoggerPtr ScLoggerFactory::defaultLogger()
{
	ScLoggerPtr& logger = ScLoggerPrivate::g_defaultLogger;
	if (!logger)
	{
		std::lock_guard<std::mutex> locker(ScLoggerFactoryData::g_factoryMutex);
		if (!logger)
		{
			ScLoggerFactoryData *&pData = ScLoggerPrivate::g_factoryData;
			if (!pData)
				pData = new ScLoggerFactoryData();
			logger = pData->createLogger();
		}
	}
	return logger;
}
