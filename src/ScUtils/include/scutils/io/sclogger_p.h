#ifndef SCLOGGER_P_H
#define SCLOGGER_P_H

#include <unordered_map>

#include "sclogger.h"
#include "spdlog/spdlog.h"
#include "spdlog/pattern_formatter.h"
#include "scutils/io/scloggerfactorybuilder.h"

using ScSpdlogLevelNames = spdlog::string_view_t[SPDLOG_LEVEL_OFF + 1];

/**
 * @brief class ScLoggerPrivate 日志类私有类
 */
class ScLoggerPrivate
{
	SC_DECLARE_PUBLIC(ScLogger)
public:
	~ScLoggerPrivate();

	ScLogger* q_ptr;	/*< q指针 */

	spdlog::logger* m_pLogger{ };		/*< spdlog日志实例 */

	static ScLoggerPtr g_defaultLogger;
	static class ScLoggerFactoryData* g_factoryData;
};

class ScLevelFormatter : public spdlog::custom_flag_formatter
{
public:
	explicit ScLevelFormatter(const ScSpdlogLevelNames& levelNames);
	~ScLevelFormatter() override = default;

	void format(const spdlog::details::log_msg& msg, const std::tm& tm_time, spdlog::memory_buf_t& dest) override;
	std::unique_ptr<spdlog::custom_flag_formatter> clone() const override;

private:
	const ScSpdlogLevelNames& levelNames;
};


/**
 * @brief class ScLoggerFactoryData 日志工厂数据类
 */
class ScLoggerFactoryData
{
public:
	/**
	 * @brief clearup 清理日志工厂实例
	 */
	static void clearup();

	/**
	 * @brief toLoggerLevel 将spdlog日志级别转换为ScLoggerLevel
	 * @param level			[in]spdlog日志级别
	 * @return Sc::LoggerLevel
	 */
	static Sc::LoggerLevel toLoggerLevel(spdlog::level::level_enum level);

	/**
	 * @brief toSpdlogLevel 将ScLoggerLevel转换为spdlog日志级别
	 * @param level			[in]ScLoggerLevel
	 * @return spdlog::level::level_enum
	 */
	static spdlog::level::level_enum toSpdlogLevel(Sc::LoggerLevel level);

	/**
	 * @brief toSpdlogTimeType 将Sc::TimeType转换为spdlog::pattern_time_type
	 * @param timeType			[in]Sc::TimeType
	 * @return spdlog::pattern_time_type
	 */
	static inline spdlog::pattern_time_type toSpdlogTimeType(Sc::TimeType timeType) {
		return Sc::TimeType::UTC == timeType ? spdlog::pattern_time_type::utc : spdlog::pattern_time_type::local;
	}

	/**
	 * @brief createSpdlog 创建spdlog日志实例
	 * @param loggerName			[in]日志名称
	 * @return spdlog日志实例
	 */
	ScLoggerPtr createLogger(const ScString& loggerName = ScString());

	ScString factoryName;		/*< 日志工厂名称 */
	ScString path;				/*< 日志存储路径 */
	ScString fileName;			/*< 日志文件名称 */
	Sc::LoggerType loggerType;	/*< 日志类型 */
	Sc::LoggerLevel loggerLevel{ Sc::LoggerLevel::Info };	/*< 日志级别 */

	int roMaxFileCount{ 5 };				/*< 回滚文件最大数目 */
	int roMaxFileSize{ 50 * 1024 * 1024 };  /*< 回滚文件大小最大字节数 */

	bool isAsyncEnabled{ false };	/*< 是否启用异步模式 */

	bool isPatternEnabled{ false };								/*< 是否启用自定义格式化器 */
	std::unique_ptr<spdlog::pattern_formatter> patternFormatter;/*< 自定义格式化器 */
	spdlog::pattern_formatter::custom_flags customFlags;		/*< 自定义格式化器标志 */
	ScLevelNames levelNames { };								/*< 自定义的日志级别名称 */
	ScSpdlogLevelNames spdlogLevelNames SPDLOG_LEVEL_NAMES;		/*< spdlog对应的日志级别名称 */
	ScString pattern {"%+"};									/*< 自定义格式化器模式 */
	spdlog::pattern_time_type timeType{ spdlog::pattern_time_type::local };		/*< 时间类型 */
	ScString eol{ SC_EOL };										/*< 换行符 */

	spdlog::sinks_init_list sinks;								/*< 日志输出回调函数列表 */

	static std::mutex g_factoryMutex;			/*< 日志工厂互斥锁 */
	static class ScLoggerFactory* g_factory;	/*< 日志工厂实例 */

	static std::mutex g_loggerMapMutex;		/*< 日志实例映射表互斥锁 */
	static std::unordered_map<ScString, ScLoggerPtr> g_loggerMap;	/*< 日志实例映射表 */
};

#endif // SCLOGGER_P_H
