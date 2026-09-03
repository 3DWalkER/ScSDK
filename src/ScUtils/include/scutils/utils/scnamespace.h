#ifndef SCNAMESPACE_H
#define SCNAMESPACE_H

#include "scutils/scglobal.h"

SC_BEGIN_NAMESPACE

/**
 * @brief The CaseSensitivity enum 字符串大小写匹配枚举
 */
enum CaseSensitivity
{
	CaseInsensitive,	/*< 不区分大小写 */
	CaseSensitive		/*< 区分大小写 */
};

/**
 * @brief The TimeType enum 事件类型
 */
enum class TimeType : scuint8
{
	Local,	/*< 本地时间 */
	UTC		/*< UTC时间 */
};

/**
 * @brief The LoggerLevel enum 日志级别
 */
enum class LoggerLevel : scuint8
{
	Off,		/*< 关闭 */
	Trace,		/*< 追溯 */
	Debug,		/*< 调试 */
	Info,		/*< 信息 */
	Warn,		/*< 警告 */
	Error,		/*< 错误 */
	Critical	/*< 致命错误 */
};

/**
 * @brief The LoggerType enum 日志输出类型
 */
enum class LoggerType : scuint8
{
	Console,    /*< 输出到控制台 */
	Basic,      /*< 存储到一个文件 */
	Rotating,   /*< 按日志大小与最大文件数进行回滚 */
	Daily,      /*< 按日期存储, 年月日 */
	Hourly      /*< 按小时存储 */
};

SC_END_NAMESPACE

#endif // SCNAMESPACE_H
