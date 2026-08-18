#ifndef SCLOGGER_P_H
#define SCLOGGER_P_H

#include "sclogger.h"
#include "spdlog/spdlog.h"
#include "spdlog/formatter.h"

/**
 * @brief class ScLoggerPrivate 日志类私有类
 */
class ScLoggerPrivate
{
	SC_DECLARE_PUBLIC(ScLogger)
public:
	~ScLoggerPrivate();

	static spdlog::level::level_enum toSpdlogLevel(Sc::LoggerLevel level);

	ScLogger* q_ptr;	/*< q指针 */

	spdlog::logger* m_pLogger{ };		/*< spdlog日志实例 */

	static class ScLoggerFactoryData *g_factoryData;
};


/**
 * @brief class ScLoggerFormatter 自定义日志格式化器
 */
class ScLoggerFormatter : public spdlog::formatter
{
public:
	void format(const spdlog::details::log_msg& msg, spdlog::memory_buf_t& dest) override;
	std::unique_ptr<spdlog::formatter> clone() const override;
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
	 * @brief createSpdlog 创建spdlog日志实例
	 * @param loggerName			[in]日志名称
	 * @return spdlog日志实例
	 */
	spdlog::logger *createSpdlog(const ScString& loggerName);

	ScString factoryName;		/*< 日志工厂名称 */
	ScString path;				/*< 日志存储路径 */
	ScString fileName;			/*< 日志文件名称 */
	Sc::LoggerType loggerType;	/*< 日志类型 */

	int roMaxFileCount{ 5 };				/*< 回滚文件最大数目 */
	int roMaxFileSize{ 50 * 1024 * 1024 };  /*< 回滚文件大小最大字节数 */

	bool isAsyncEnabled{ false };	/*< 是否启用异步模式 */

	static std::mutex g_factoryMutex;			/*< 日志工厂互斥锁 */
	static class ScLoggerFactory* g_factory;	/*< 日志工厂实例 */

	static std::mutex g_loggerMapMutex;		/*< 日志实例映射表互斥锁 */
	static std::unordered_map<ScString, ScLoggerPtr> g_loggerMap;	/*< 日志实例映射表 */
};

#endif // SCLOGGER_P_H
