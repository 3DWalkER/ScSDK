#ifndef SCAPPLICATION_H
#define SCAPPLICATION_H

#include "scutils/scglobal.h"
#include <string>

class ScApplicationPrivate;

class SC_API_EXPORT ScApplication
{
	SC_DECLARE_PRIVATE(ScApplication)
public:
    ScApplication(int argc, char **argv);
	~ScApplication();

    /**
     * @brief applicationDirPath 获取程序所在路径
     */
    static std::string applicationDirPath();

	/**
	 * @brief self 程序实例
	 */
	static ScApplication *instance() { return self;  }

	/**
	 * @brief exec 执行程序
	 */
	static int exec();

    /**
     * @brief quit 退出程序
     */
    static void quit();

protected:
    ScApplication(ScApplicationPrivate *q);

private:
	ScApplicationPrivate *d_ptr;

	/**
	 * @brief self 程序实例
	 */
	static ScApplication *self;
};

#endif // SCAPPLICATION_H
