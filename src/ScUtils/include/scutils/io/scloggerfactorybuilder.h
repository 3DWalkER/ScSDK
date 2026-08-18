#ifndef SCLOGGERFACTORYBUILDER_H
#define SCLOGGERFACTORYBUILDER_H

#include"scutils/utils/scnamespace.h"

class ScLoggerFactory;
class ScLoggerFactoryData;

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
	 * @brief setAsyncEnabled 设置是否启用异步模式
	 * @param 
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
	 * @brief build 构建日志工厂
	 * @return 日志工厂实例
	 */
	ScLoggerFactory* build();

private:
	ScLoggerFactoryData* d;	/*< 日志工厂数据实例 */
};

#endif // SCLOGGERFACTORYBUILDER_H
