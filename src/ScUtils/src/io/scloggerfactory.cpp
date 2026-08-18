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
			ScLogger* pLogger = new ScLogger();
			pLogger->d_func()->m_pLogger = d->createSpdlog(loggerName);
			pLogger->setLevel(Sc::LoggerLevel::Trace);
			ScLoggerPtr loggerPtr = ScLoggerPtr(pLogger, [](ScLogger *p) {
				delete p;
			});
			d->g_loggerMap[loggerName] = loggerPtr;
			return loggerPtr;
		}
	}
	return it->second;
}

ScLoggerPtr ScLoggerFactory::defaultLogger()
{
	return nullptr;
}
