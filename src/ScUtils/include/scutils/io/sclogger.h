#ifndef SCLOGGER_H
#define SCLOGGER_H

#include "scutils/io/scformat.h"
#include "scutils/io/scloggerfactory.h"

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
	friend class ScLoggerFactoryData;
};

SC_BEGIN_NAMESPACE

SC_BEGIN_DETAIL_NAMESPACE

template <typename... Args>
ScString joinHelper(std::true_type, Args &&...args)
{
	return {};
}

template <typename... Args>
ScString joinHelper(std::false_type, Args &&...args)
{
	ScString result;
	SC_CONSTEXPR char prefix[] = " with parameters:";
	SC_CONSTEXPR size_t prefixLength = sizeof(prefix) - 1;
	const size_t size = Sc::formatted_size(std::forward<Args>(args)...) + sizeof...(Args) + prefixLength;
	result.reserve(size);
	result.append(prefix, prefixLength);
	int dummy[] = {
		0,
		(
			[&]() {
				result += ' ';
				fmt::format_to(std::back_inserter(result), "{}", std::forward<Args>(args));
			}(),
			0
		)...
	};
	SC_UNUSED(dummy);
	return result;
}

template <typename... Args>
ScString joinArgs(const ScString& prefix, const ScString& funcName, Args &&...args)
{
	using is_zero = typename std::conditional<(sizeof...(Args) == 0),
		std::true_type, std::false_type
	>::type;
	const ScString& parameters = joinHelper(is_zero{ }, std::forward<Args>(args)...);
	return Sc::format("[{}]{}{}", funcName, prefix, parameters);
}

template <typename... Args>
ScString joinArgsIn(const ScString& funcName, Args &&...args) {
	return joinArgs("Entering", funcName, std::forward<Args>(args)...);
}

template <typename... Args>
ScString joinArgsOut(const ScString& funcName, Args &&...args) {
	return joinArgs("Exiting", funcName, std::forward<Args>(args)...);
}

SC_END_DETAIL_NAMESPACE

SC_END_NAMESPACE

#define __SC_LOGGER_MAKE_IN__(...) SC_DETAIL::joinArgsIn(__FUNCTION__, __VA_ARGS__)

#define __SC_LOGGER_MAKE_OUT__(...) SC_DETAIL::joinArgsOut(__FUNCTION__, __VA_ARGS__)

#define SC_LOGGER_TRACE_IN(logger, ...) \
	SC_LOGGER_TRACE(logger, __SC_LOGGER_MAKE_IN__(__VA_ARGS__));

#define SC_LOGGER_TRACE_OUT(logger, ...) \
	SC_LOGGER_TRACE(logger, __SC_LOGGER_MAKE_OUT__(__VA_ARGS__));

#define SC_TRACE_IN(...) \
	SC_LOGGER_TRACE_IN(SC_LOGGER_DEFAULT, __VA_ARGS__)

#define SC_TRACE_OUT(...) \
	SC_LOGGER_TRACE_OUT(SC_LOGGER_DEFAULT, __VA_ARGS__)

#define SC_LOGGER_DEBUG_IN(logger, ...) \
	SC_LOGGER_DEBUG(logger, __SC_LOGGER_MAKE_IN__(__VA_ARGS__));

#define SC_LOGGER_DEBUG_OUT(logger, ...) \
	SC_LOGGER_DEBUG(logger, __SC_LOGGER_MAKE_OUT__(__VA_ARGS__));

#define SC_DEBUG_IN(...) \
	SC_LOGGER_DEBUG_IN(SC_LOGGER_DEFAULT, __VA_ARGS__)

#define SC_DEBUG_OUT(...) \
	SC_LOGGER_DEBUG_OUT(SC_LOGGER_DEFAULT, __VA_ARGS__)

#define SC_LOGGER_INFO_IN(logger, ...) \
	SC_LOGGER_INFO(logger, __SC_LOGGER_MAKE_IN__(__VA_ARGS__));

#define SC_LOGGER_INFO_OUT(logger, ...) \
	SC_LOGGER_INFO(logger, __SC_LOGGER_MAKE_OUT__(__VA_ARGS__));

#define SC_INFO_IN(...) \
	SC_LOGGER_INFO_IN(SC_LOGGER_DEFAULT, __VA_ARGS__)

#define SC_INFO_OUT(...) \
	SC_LOGGER_INFO_OUT(SC_LOGGER_DEFAULT, __VA_ARGS__)

#define SC_LOGGER_WARN_IN(logger, ...) \
	SC_LOGGER_WARN(logger, __SC_LOGGER_MAKE_IN__(__VA_ARGS__));

#define SC_LOGGER_WARN_OUT(logger, ...) \
	SC_LOGGER_WARN(logger, __SC_LOGGER_MAKE_OUT__(__VA_ARGS__));

#define SC_WARN_IN(...) \
	SC_LOGGER_WARN_IN(SC_LOGGER_DEFAULT, __VA_ARGS__)

#define SC_WARN_OUT(...) \
	SC_LOGGER_WARN_OUT(SC_LOGGER_DEFAULT, __VA_ARGS__)

#define SC_LOGGER_ERROR_IN(logger, ...) \
	SC_LOGGER_ERROR(logger, __SC_LOGGER_MAKE_IN__(__VA_ARGS__));

#define SC_LOGGER_ERROR_OUT(logger, ...) \
	SC_LOGGER_ERROR(logger, __SC_LOGGER_MAKE_OUT__(__VA_ARGS__));

#define SC_ERROR_IN(...) \
	SC_LOGGER_ERROR_IN(SC_LOGGER_DEFAULT, __VA_ARGS__)

#define SC_ERROR_OUT(...) \
	SC_LOGGER_ERROR_OUT(SC_LOGGER_DEFAULT, __VA_ARGS__)

#define SC_LOGGER_CRITICAL_IN(logger, ...) \
	SC_LOGGER_CRITICAL(logger, __SC_LOGGER_MAKE_IN__(__VA_ARGS__));

#define SC_LOGGER_CRITICAL_OUT(logger, ...) \
	SC_LOGGER_CRITICAL(logger, __SC_LOGGER_MAKE_OUT__(__VA_ARGS__));

#define SC_CRITICAL_IN(...) \
	SC_LOGGER_CRITICAL_IN(SC_LOGGER_DEFAULT, __VA_ARGS__)

#define SC_CRITICAL_OUT(...) \
	SC_LOGGER_CRITICAL_OUT(SC_LOGGER_DEFAULT, __VA_ARGS__)

#endif // SCLOGGER_H
