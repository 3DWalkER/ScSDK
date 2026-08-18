#ifndef SCLOGGERFACTORY_H
#	include "scutils/io/scloggerfactory.h"
#endif

#ifndef SCLOGGER_H
#define SCLOGGER_H

#include "scutils/io/scformat.h"
#include "scutils/utils/scnamespace.h"

class ScLoggerPrivate;

class SC_API_EXPORT ScLogger
{
	SC_DECLARE_PRIVATE(ScLogger)
public:
	/**
	 * @brief setType 设置日志级别，默认为Trace
	 * @param level			[in]日志级别
	 */
	void setLevel(Sc::LoggerLevel level);

	/**
	 * @brief trace 追溯日志
	 * @tparam Args 日志参数类型
	 */
	template <typename... Args>
	inline void trace(const ScString& msg, Args &&...args) {
		log(Sc::LoggerLevel::Trace, msg, std::forward<Args>(args)...);
	}

	/**
	 * @brief information 输出运行信息
	 */
	template <typename... Args>
	inline void info(const ScString& msg, Args &&...args) {
		log(Sc::LoggerLevel::Info, msg, std::forward<Args>(args)...);
	}

	/**
	 * @brief debug 输出调试信息
	 */
	template <typename... Args>
	inline void debug(const ScString& msg, Args &&...args) {
		log(Sc::LoggerLevel::Debug, msg, std::forward<Args>(args)...);
	}

	/**
	 * @brief warning 输出警告信息
	 */
	template <typename... Args>
	inline void warning(const ScString& msg, Args &&...args) {
		log(Sc::LoggerLevel::Warn, msg, std::forward<Args>(args)...);
	}

	/**
	 * @brief error 输出错误信息
	 */
	template <typename... Args>
	inline void error(const ScString& msg, Args &&...args) {
		log(Sc::LoggerLevel::Error, msg, std::forward<Args>(args)...);
	}

	/**
	 * @brief critical 输出严重信息
	 */
	template <typename... Args>
	inline void critical(const ScString& msg, Args &&...args) {
		log(Sc::LoggerLevel::Critical, msg, std::forward<Args>(args)...);
	}

	/**
	 * @brief flush 刷新日志缓冲区
	 */
	void flush();
	
private:
	ScLogger();
	~ScLogger();

	/**
	 * @brief log 日志输出
	 */
	void _log(Sc::LoggerLevel level, const ScString& message);

	/**
	 * @brief log 日志输出
	 * @tparam Args 日志参数类型
	 */
	template <typename... Args>
	inline void log(Sc::LoggerLevel level, const ScString& msg, Args &&...args) {
		_log(level, Sc::format(msg, std::forward<Args>(args)...));
	}

	ScLoggerPrivate* d_ptr;
	friend class ScLoggerFactory;
};

using ScLoggerPtr = std::shared_ptr<ScLogger>;

#endif // SCLOGGER_H
