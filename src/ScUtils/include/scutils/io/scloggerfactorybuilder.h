#ifndef SCLOGGERFACTORYBUILDER_H
#define SCLOGGERFACTORYBUILDER_H

#include "scutils/text/scstring.h"
#include <vector>
#include <functional>

class ScLoggerFactory;
class ScLoggerFactoryData;

/**
 * @brief ScLevelNames 日志级别名称向量
 */
using ScLevelNames = std::vector<std::pair<Sc::LoggerLevel, ScString>>;

/**
 * @brief ScSinkCallback 日志输出回调函数
 */
using ScSinkCallback = std::function<void(const ScStringView &)>;

/**
 * @brief class ScLoggerFactoryBuilder 日志工厂构建器类
 */
class SC_API_EXPORT ScLoggerFactoryBuilder
{
public:
	ScLoggerFactoryBuilder(ScString factoryName, ScString path, ScString fileName);
	~ScLoggerFactoryBuilder();

	/**
	 * @brief setLoggerType 设置日志输出类型，默认为LoggerType::Daily
	 * @param type				[in]日志输出类型
	 * @return 日志工厂构建器实例
	 */
	ScLoggerFactoryBuilder& setLoggerType(Sc::LoggerType type);

	/**
	 * @brief setLoggerLevel 设置日志级别
	 * @param level				[in]日志工厂默认的日志级别
	 */
	ScLoggerFactoryBuilder& setLoggerLevel(Sc::LoggerLevel level);

	/**
	 * @brief setAsyncEnabled 设置是否启用异步模式
	 * @param on				[in]是否启用异步模式，ture：启用
	 */
	ScLoggerFactoryBuilder& setAsyncEnabled(bool on);

	/**
	 * @brief setRoatingMaxFileSize 设置回滚日志文件最大字节数
	 * @param size				[in]最大字节数
	 * @return 日志工厂构建器实例
	 */
	ScLoggerFactoryBuilder& setRoatingMaxFileSize(int size);

	/**
	 * @brief setRoatingMaxFileCount 设置回滚日志文件最大数目
	 * @param count				[in]最大数目
	 * @return 日志工厂构建器实例
	 */
	ScLoggerFactoryBuilder& setRoatingMaxFileCount(int count);

	/**
	 * @brief setPattern 设置日志输出格式
	 * @param pattern				[in]日志输出格式
	 * @param timeType				[in]时间类型，默认为Local
	 * @return 日志工厂构建器实例
	 */
	ScLoggerFactoryBuilder& setPattern(ScString pattern, Sc::TimeType timeType = Sc::TimeType::Local, ScString eol = SC_EOL);

	/** 
	 * @brief setLevelPattern 设置日志级别格式
	 * @param levelNames				[in]日志级别格式
	 * @return 日志工厂构建器实例
	 */
	ScLoggerFactoryBuilder& setLevelPattern(ScLevelNames levelNames);

	/**
	 * @brief addSink 添加日志输出回调函数
	 * @param callback				[in]日志输出回调函数
	 * @return 日志工厂构建器实例
	 */
	ScLoggerFactoryBuilder& addSink(ScSinkCallback callback);

	/**
	 * @brief build 构建日志工厂
	 * @return 日志工厂实例
	 */
	ScLoggerFactory* build();

	/**
	 * @brief buildDefault 构建默认的日志数据
	 */
	void buildDefault();

private:
	ScLoggerFactoryData* d;	/*< 日志工厂数据实例 */
};

#endif // SCLOGGERFACTORYBUILDER_H
