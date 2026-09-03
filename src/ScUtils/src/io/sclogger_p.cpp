#include "scutils/io/sclogger_p.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/hourly_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/details/fmt_helper.h"
#include "spdlog/async.h"

ScLoggerPtr ScLoggerPrivate::g_defaultLogger = nullptr;
ScLoggerFactoryData* ScLoggerPrivate::g_factoryData = nullptr;

std::mutex ScLoggerFactoryData::g_factoryMutex;
ScLoggerFactory* ScLoggerFactoryData::g_factory = nullptr;

std::mutex ScLoggerFactoryData::g_loggerMapMutex;
std::unordered_map<ScString, ScLoggerPtr> ScLoggerFactoryData::g_loggerMap;

ScLoggerPtr ScLoggerFactoryData::createLogger(const ScString& loggerName)
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

	auto spdlog = [this, loggerName, sink]() -> spdlog::logger* {
		ScString fullLoggerName = factoryName;
		if (!loggerName.isEmpty())
			fullLoggerName += "/" + loggerName;

		ScSinks sinks = this->sinks;
		sinks.push_back(sink);

		if (!isAsyncEnabled)
			return new spdlog::logger(fullLoggerName.data(), sinks.begin(), sinks.end());
		return new spdlog::async_logger(fullLoggerName.data(), sinks.begin(), sinks.end(), spdlog::thread_pool());
	}();

	ScLogger* pLogger = new ScLogger();
	pLogger->d_func()->m_pLogger = spdlog;
	pLogger->setLevel(loggerLevel);
	if (patternFormatter)
		spdlog->set_formatter(patternFormatter->clone());
	return ScLoggerPtr(pLogger, [](ScLogger* p) { delete p; });
}

void ScLoggerFactoryData::setupFormatter()
{
	if (isPatternEnabled)
	{
		if (isFullFormatter() && !customFlags.empty())
			pattern = "%Y-%m-%d %H:%M:%S.%e %a [%n] [%^%l%$] %v";

		patternFormatter = spdlog::details::make_unique<spdlog::pattern_formatter>(
			pattern.data(), timeType, eol.data(), std::move(customFlags)
		);
	}
}

void ScLoggerFactoryData::clearup()
{
	if (!g_factory)
		return;

	spdlog::shutdown();

	delete g_factory;
	g_factory = nullptr;
}

Sc::LoggerLevel ScLoggerFactoryData::toLoggerLevel(spdlog::level::level_enum level)
{
	switch (level)
	{
	case spdlog::level::trace:
		return Sc::LoggerLevel::Trace;
	case spdlog::level::info:
		return Sc::LoggerLevel::Info;
	case spdlog::level::debug:
		return Sc::LoggerLevel::Debug;
	case spdlog::level::warn:
		return Sc::LoggerLevel::Warn;
	case spdlog::level::err:
		return Sc::LoggerLevel::Error;
	case spdlog::level::critical:
		return Sc::LoggerLevel::Critical;
	default:
		return Sc::LoggerLevel::Off;
	}
}

spdlog::level::level_enum ScLoggerFactoryData::toSpdlogLevel(Sc::LoggerLevel level)
{
	switch (level)
	{
	case Sc::LoggerLevel::Trace:
		return spdlog::level::trace;
	case Sc::LoggerLevel::Info:
		return spdlog::level::info;
	case Sc::LoggerLevel::Debug:
		return spdlog::level::debug;
	case Sc::LoggerLevel::Warn:
		return spdlog::level::warn;
	case Sc::LoggerLevel::Error:
		return spdlog::level::err;
	case Sc::LoggerLevel::Critical:
		return spdlog::level::critical;
	default:
		return spdlog::level::off;
	}
}

static void cleanupLoggerFactory()
{
	ScLoggerFactoryData::clearup();
}
SC_DESTRUCTOR_FUNCTION(cleanupLoggerFactory)

ScLevelFormatter::ScLevelFormatter(const ScSpdlogLevelNames& levelNames)
	: levelNames(levelNames)
{

}

std::unique_ptr<spdlog::custom_flag_formatter> ScLevelFormatter::clone() const
{
	return spdlog::details::make_unique<ScLevelFormatter>(levelNames);
}

void ScLevelFormatter::format(const spdlog::details::log_msg& msg, const std::tm&, spdlog::memory_buf_t& dest)
{
	const spdlog::string_view_t& level_name = levelNames[msg.level];
	spdlog::details::fmt_helper::append_string_view(level_name, dest);
}

ScBasicSink::ScBasicSink(ScSinkCallback callback)
	: spdlog::sinks::base_sink<std::mutex>()
	, m_callback(std::move(callback))
{
}

void ScBasicSink::sink_it_(const spdlog::details::log_msg& msg)
{
	if (m_callback)
	{
		spdlog::memory_buf_t formatted;
		base_sink<std::mutex>::formatter_->format(msg, formatted);
		m_callback(ScStringView(formatted.data(), formatted.size()));
	}
}

void ScBasicSink::flush_()
{
}
