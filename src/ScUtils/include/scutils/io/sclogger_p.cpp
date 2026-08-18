#include "sclogger_p.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/hourly_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/async.h"

ScLoggerFactoryData * ScLoggerPrivate::g_factoryData = nullptr;

std::mutex ScLoggerFactoryData::g_factoryMutex;
ScLoggerFactory* ScLoggerFactoryData::g_factory = nullptr;

std::mutex ScLoggerFactoryData::g_loggerMapMutex;
std::unordered_map<ScString, ScLoggerPtr> ScLoggerFactoryData::g_loggerMap;

void ScLoggerFactoryData::clearup()
{
	if (!g_factory)
		return;

	spdlog::shutdown();

	delete g_factory;
	g_factory = nullptr;
}

static void cleanupLoggerFactory()
{
	ScLoggerFactoryData::clearup();
}
SC_DESTRUCTOR_FUNCTION(cleanupLoggerFactory)

spdlog::logger* ScLoggerFactoryData::createSpdlog(const ScString& loggerName)
{
	spdlog::sink_ptr sink;
	const ScString fileFullName = path + "/" + fileName;
	switch (loggerType)
	{
	case Sc::LoggerType::Basic:
		sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileFullName.data());
		break;
	case Sc::LoggerType::Rotating:
		sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(fileFullName.data(), roMaxFileSize, roMaxFileCount);
		break;
	case Sc::LoggerType::Daily:
		sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(fileFullName.data(), 0, 0);
		break;
	case Sc::LoggerType::Hourly:
		sink = std::make_shared<spdlog::sinks::hourly_file_sink_mt>(fileFullName.data());
		break;
	default:
		sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		break;
	}

	std::string fullLoggerName = Sc::format("{}/{}", factoryName, loggerName).data();
	if (!isAsyncEnabled)
		return new spdlog::logger(fullLoggerName, std::move(sink));
	return new spdlog::async_logger(fullLoggerName, sink, spdlog::thread_pool());
}