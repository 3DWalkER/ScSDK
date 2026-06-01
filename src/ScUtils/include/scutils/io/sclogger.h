#ifndef CDLOGGER_H
#define CDLOGGER_H

#include "scutils/io/scformat.h"

class ScLoggerPrivate;

class SC_API_EXPORT ScLogger
{
	SC_DECLARE_PRIVATE(ScLogger)
public:
	/**
	 * @brief The Type enum 日志输出方式
	 */
	enum Type
	{
		Console,    /*< 输出到控制台 */
		Basic,      /*< 存储到一个文件 */
		Rotating,   /*< 按日志大小与最大文件数进行回滚 */
		Daily,      /*< 按日期存储, 年月日 */
		Hourly      /*< 按小时存储 */
	};

	/**
	 * @brief The Level enum 日志级别
	 */
	enum Level
	{
		Off,		/*< 关闭 */
		Trace,		/*< 追溯 */
		Info,		/*< 信息 */
		Debug,		/*< 调试 */
		Warn,		/*< 警告 */
		Error,		/*< 错误 */
		Critical	/*< 致命错误 */
	};

	/**
	 * @brief setType 设置日志输出类型，默认为Console
	 * @param type			[in]日志类型
	 */
	void setType(Type type);

	/**
	 * @brief setType 设置日志级别，默认为Trace
	 * @param level			[in]日志级别
	 */
	void setLevel(Level level);

	/**
	 * @brief debug 输出追溯信息
	 */
	template <typename... Args>
	inline void trace(const ScString &msg, Args &&...args) {
		log(Trace, msg, std::forward<Args>(args)...);
	}

	/**
	 * @brief debug 输出调试信息
	 */
	template <typename... Args>
	inline void debug(const ScString &msg, Args &&...args) {
		log(Debug, msg, std::forward<Args>(args)...);
	}

	/**
	 * @brief information 输出运行信息
	 */
	template <typename... Args>
	inline void information(const ScString &msg, Args &&...args) {
		log(Info, msg, std::forward<Args>(args)...);
	}

	/**
	 * @brief warning 输出警告信息
	 */
	template <typename... Args>
	inline void warning(const ScString &msg, Args &&...args) {
		log(Warn, msg, std::forward<Args>(args)...);
	}

	/**
	 * @brief error 输出错误信息
	 */
	template <typename... Args>
	inline void error(const ScString &msg, Args &&...args) {
		log(Error, msg, std::forward<Args>(args)...);
	}

	/**
	 * @brief critical 输出严重信息
	 */
	template <typename... Args>
	inline void critical(const ScString &msg, Args &&...args) {
		log(Critical, msg, std::forward<Args>(args)...);
	}

	/**
	 * @brief instance 获取日志单例
	 */
	static ScLogger *instance();

private:
    explicit ScLogger(Type type = Console);
	~ScLogger();

	/**
	 * @brief log 记录日志信息
	 */
	void _log(Level level, const ScString &message);

	/**
	 * @brief log 安全的记录日志信息
	 */
	template <typename... Args>
	inline void log(Level level, const ScString &msg, Args &&...args) {
		_log(level, Sc::format(msg, std::forward<Args>(args)...));
	}

	/**
	 * @brief d_ptr d指针
	 */
	ScLoggerPrivate *d_ptr;
};

#define SC_LOGGER ScLogger::instance()
#define SC_TRACE(Msg, ...)      SC_LOGGER->trace(Msg, ##__VA_ARGS__)
#define SC_DEBUG(Msg, ...)      SC_LOGGER->debug(Msg, ##__VA_ARGS__)
#define SC_INFO(Msg, ...)       SC_LOGGER->information(Msg, ##__VA_ARGS__)
#define SC_WARN(Msg, ...)       SC_LOGGER->warning(Msg, ##__VA_ARGS__)
#define SC_ERROR(Msg, ...)      SC_LOGGER->error(Msg, ##__VA_ARGS__)
#define SC_CRITICAL(Msg, ...)   SC_LOGGER->critical(Msg, ##__VA_ARGS__)

#endif // CDLOGGER_H
