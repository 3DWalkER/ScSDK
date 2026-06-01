#include "scutils/io/sclogger.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/hourly_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <cstdarg>

class ScCustomFormatter : public spdlog::formatter
{
public:
	void format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) override
	{
		static std::unordered_map<spdlog::level::level_enum, std::pair<std::string, std::string>> levelMapping = {
			{ spdlog::level::trace,		{ "追踪", "gray" } },
			{ spdlog::level::debug,		{ "调试", "blue" } },
			{ spdlog::level::info,		{ "运行", "green"} },
			{ spdlog::level::warn,		{ "警告", "orange"} },
			{ spdlog::level::err,		{ "错误", "red"} },
			{ spdlog::level::critical,	{ "严重", "darkred"} },
			{ spdlog::level::off,		{ "关闭", "black"} }
		};

		auto level_info = levelMapping[msg.level];
		std::string color = level_info.second;
		std::string level_chinese = level_info.first;

		time_t curr = std::chrono::system_clock::to_time_t(msg.time);;
		struct tm *local_time = localtime(&curr);
		char timestr[24]{ };
		strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %a", local_time);

		auto fmt = fmt::format_string<std::string>("<a style='color:{}'>【{} | {}】{}</a><br/>");
		fmt::format_to(std::back_inserter(dest), fmt, color, level_chinese, timestr, msg.payload);
	}

	std::unique_ptr<formatter> clone() const override
	{
        return spdlog::details::make_unique<ScCustomFormatter>();
	}
};

class ScLoggerPrivate
{
	SC_DECLARE_PUBLIC(ScLogger)
public:
	ScLoggerPrivate(ScLogger *q, ScLogger::Type type);

	/**
	 * @brief updateLogger 更新日志管理器
	 */
	void updateLogger();

	/**
	 * @brief create 创建日志文件
	 */
	static std::string create();

	/**
	 * @brief create 将日志级别转换为spdlog库的日志级别
	 */
	spdlog::level::level_enum toSpdlogLevel(ScLogger::Level level);

	/**
	 * @brief q_ptr q指针
	 */
	ScLogger *q_ptr;

	/**
	 * @brief m_pLogger spdlog日志
	 */
	std::shared_ptr<spdlog::logger> m_pLogger;

	/**
	 * @brief outType 日志输出方式
	 */
	ScLogger::Type outType;

	/**
	 * @brief level 日志级别
	 */
	ScLogger::Level level = ScLogger::Trace;

	/**
	 * @brief spdlogLv 日志级别
	 */
	spdlog::level::level_enum spdlogLv = spdlog::level::info;

	/**
	 * @brief loggerName 日志名称
	 */
	std::string loggerName;

	/**
	 * @brief fileName 日志存储文件名
	 */
	std::string fileName;

	/**
	 * @brief roMaxFileSize 回滚文件大小最大值
	 */
	int roMaxFileSize;

	/**
	 * @brief roMaxFileCount 回滚文件最大数目
	 */
	int roMaxFileCount;

	/**
	 * @brief instance 日志实例
	 */
	static ScLogger *instance;

	/**
	 * @brief instanceLocker 多线程使用实例时使用的保护锁
	 */
	static std::mutex instanceLocker;
};

std::mutex ScLoggerPrivate::instanceLocker;
ScLogger *ScLoggerPrivate::instance = nullptr;

ScLogger::ScLogger(ScLogger::Type type)
	: d_ptr(new ScLoggerPrivate(this, type))
{
	
}

ScLogger::~ScLogger()
{
	delete d_ptr;
}

void ScLogger::setType(ScLogger::Type type)
{
	SC_D(ScLogger);
	if (type == d->outType)
		return;

	d->outType = type;
	d->updateLogger();
}

void ScLogger::setLevel(ScLogger::Level level)
{
	SC_D(ScLogger);
	if (nullptr == d->m_pLogger || d->level == level)
		return;

	d->level = level;
	d->spdlogLv = d->toSpdlogLevel(level);
	d->m_pLogger->set_level(d->spdlogLv);
}

void ScLogger::_log(Level level, const ScString &message)
{
	SC_D(ScLogger);
	d->m_pLogger->log(d->toSpdlogLevel(level), message);
	d->m_pLogger->flush();
}

ScLogger *ScLogger::instance()
{
	if (nullptr == ScLoggerPrivate::instance)
	{
		std::unique_lock<std::mutex> lock(ScLoggerPrivate::instanceLocker);
		if (nullptr == ScLoggerPrivate::instance)
			ScLoggerPrivate::instance = new ScLogger();
	}
	return ScLoggerPrivate::instance;
}


ScLoggerPrivate::ScLoggerPrivate(ScLogger *q, ScLogger::Type type)
	: q_ptr(q)
	, outType(type)
	, loggerName("SC_LOGGER_NAME_2024_12_28")
    , fileName(create())
	, roMaxFileSize(1048576)
	, roMaxFileCount(5)
{
	updateLogger();
}

void ScLoggerPrivate::updateLogger()
{
	switch (outType)
	{
	case ScLogger::Console:
		m_pLogger = spdlog::stdout_color_mt("Console");
		m_pLogger->set_level(spdlogLv);
		return;
	case ScLogger::Basic:
		m_pLogger = spdlog::basic_logger_mt(loggerName, fileName);
		break;
	case ScLogger::Daily:
		m_pLogger = spdlog::daily_logger_mt(loggerName, fileName);
		break;
	case ScLogger::Hourly:
		m_pLogger = spdlog::hourly_logger_mt(loggerName, fileName);
		break;
	case ScLogger::Rotating:
		m_pLogger = spdlog::rotating_logger_mt(loggerName, fileName, static_cast<size_t>(roMaxFileSize), static_cast<size_t>(roMaxFileCount));
		break;
	default:
		m_pLogger = spdlog::basic_logger_mt(loggerName, fileName);
		m_pLogger->set_level(spdlog::level::off);
		return;
	}

	m_pLogger->set_level(spdlogLv);
    m_pLogger->set_formatter(spdlog::details::make_unique<ScCustomFormatter>());
}

std::string ScLoggerPrivate::create()
{
	//std::string filePath = ScApplication::applicationDirPath() + "/log";
	//if (!ScDir::isExists(filePath))
	//{
	//	if (!ScDir::mkpath(filePath) || !ScDir::isReadWritable(filePath))
	//		return "";
	//}

	//return filePath + "/log.html";
	return std::string();
}

spdlog::level::level_enum ScLoggerPrivate::toSpdlogLevel(ScLogger::Level level)
{
	switch (level)
	{
	case ScLogger::Trace:
		return spdlog::level::trace;
	case ScLogger::Info:
		return spdlog::level::info;
	case ScLogger::Debug:
		return spdlog::level::debug;
	case ScLogger::Warn:
		return spdlog::level::warn;
	case ScLogger::Error:
		return spdlog::level::err;
	case ScLogger::Critical:
		return spdlog::level::critical;
	default:
		return spdlog::level::off;
	}
}
