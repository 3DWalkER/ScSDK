#ifndef SCLOGGERFACTORY_H
#define SCLOGGERFACTORY_H

#include "scutils/io/sclogger.h"

class ScLoggerFactoryData;

class SC_API_EXPORT ScLoggerFactory
{
public:
	ScLoggerPtr logger(const ScString& loggerName);

	static ScLoggerPtr defaultLogger();

private:
	explicit ScLoggerFactory(ScLoggerFactoryData* d);
	~ScLoggerFactory();

	ScLoggerFactoryData* d;		/*< 日志工厂数据实例 */
	friend class ScLoggerFactoryData;
	friend class ScLoggerFactoryBuilder;
};

#define SC_LOGGER_LEVEL_TRANCE		0
#define SC_LOGGER_LEVEL_DEBUG		1
#define SC_LOGGER_LEVEL_INFO		2
#define SC_LOGGER_LEVEL_WARN		3
#define SC_LOGGER_LEVEL_ERROR		4
#define SC_LOGGER_LEVEL_CRITICAL	5
#define SC_LOGGER_LEVEL_OFF			6

#ifndef SC_LOGGER_ACTIVE_LEVEL
#	define SC_LOGGER_ACTIVE_LEVEL SC_LOGGER_LEVEL_INFO
#endif

#if SC_LOGGER_ACTIVE_LEVEL <= SC_LOGGER_LEVEL_TRANCE
#	define SC_LOGGER_TRANCE(logger, msg, ...) \
		logger->trace(msg, __VA_ARGS__);
#	define SC_TRANCE(msg, ...) SC_LOGGER_TRANCE(ScLoggerFactory::defaultLogger())
#else
#	define SC_LOGGER_TRANCE(logger, msg, ...) (void)0;
#	define SC_TRANCE(msg, ...) (void)0;
#endif

#if SC_LOGGER_ACTIVE_LEVEL <= SC_LOGGER_LEVEL_DEBUG
#	define SC_LOGGER_DEBUG(logger, msg, ...) \
		logger->debug(msg, __VA_ARGS__);
#	define SC_DEBUG(msg, ...) SC_LOGGER_TRANCE(ScLoggerFactory::defaultLogger())
#else
#	define SC_LOGGER_DEBUG(logger, msg, ...) (void)0;
#	define SC_DEBUG(msg, ...) (void)0;
#endif

#if SC_LOGGER_ACTIVE_LEVEL <= SC_LOGGER_LEVEL_INFO
#	define SC_LOGGER_INFO(logger, msg, ...) \
		logger->info(msg, __VA_ARGS__);
#	define SC_INFO(msg, ...) SC_LOGGER_TRANCE(ScLoggerFactory::defaultLogger())
#else
#	define SC_LOGGER_INFO(logger, msg, ...) (void)0;
#	define SC_INFO(msg, ...) (void)0;
#endif

#if SC_LOGGER_ACTIVE_LEVEL <= SC_LOGGER_LEVEL_WARN
#	define SC_LOGGER_WARN(logger, msg, ...) \
		logger->warning(msg, __VA_ARGS__);
#	define SC_WARN(msg, ...) SC_LOGGER_TRANCE(ScLoggerFactory::defaultLogger())
#else
#	define SC_LOGGER_WARN(logger, msg, ...) (void)0;
#	define SC_WARN(msg, ...) (void)0;
#endif

#if SC_LOGGER_ACTIVE_LEVEL <= SC_LOGGER_LEVEL_ERROR
#	define SC_LOGGER_ERROR(logger, msg, ...) \
		logger->error(msg, __VA_ARGS__);
#	define SC_ERROR(msg, ...) SC_LOGGER_TRANCE(ScLoggerFactory::defaultLogger())
#else
#	define SC_LOGGER_ERROR(logger, msg, ...) (void)0;
#	define SC_ERROR(msg, ...) (void)0;
#endif

#if SC_LOGGER_ACTIVE_LEVEL <= SC_LOGGER_LEVEL_ERROR
#	define SC_LOGGER_ERROR(logger, msg, ...) \
		logger->error(msg, __VA_ARGS__);
#	define SC_ERROR(msg, ...) SC_LOGGER_TRANCE(ScLoggerFactory::defaultLogger())
#else
#	define SC_LOGGER_ERROR(logger, msg, ...) (void)0;
#	define SC_ERROR(msg, ...) (void)0;
#endif

#if SC_LOGGER_ACTIVE_LEVEL <= SC_LOGGER_LEVEL_CRITICAL
#	define SC_LOGGER_CRITICAL(logger, msg, ...) \
		logger->critical(msg, __VA_ARGS__);
#	define SC_CRITICAL(msg, ...) SC_LOGGER_TRANCE(ScLoggerFactory::defaultLogger())
#else
#	define SC_LOGGER_CRITICAL(logger, msg, ...) (void)0;
#	define SC_CRITICAL(msg, ...) (void)0;
#endif

#endif // SCLOGGERFACTORY_H
