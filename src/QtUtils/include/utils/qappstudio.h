#ifndef QAPPSTUDIO_H
#define QAPPSTUDIO_H

#include "qutilsglobal.h"

class QPluginManager;
class QAppStudioPrivate;
class QAbstractConfiguration;
class QSqlDatabaseManager;
class QtAdvancedStylesheet;

class QU_API_EXPORT QAppStudio
{
    Q_DECLARE_SINGLETON(QAppStudio)
public:
    ~QAppStudio();

    /**
     * @brief configuration 配置文件存储管理器
     */
    QAbstractConfiguration *configuration() const;
    void setConfiguration(QAbstractConfiguration *config);

    /**
     * @brief pluginManager 插件管理器
     */
    QPluginManager *pluginManager() const;
    void setePluginManager(QPluginManager *mgr);

    /**
     * @brief databaseManager 数据库管理器
     */
    QSqlDatabaseManager *databaseManager() const;
    void setDatabaseManager(QSqlDatabaseManager *mgr);

    /**
     * @brief advancedStylesheet qss样式表管理器
     */
    QtAdvancedStylesheet *advancedStylesheet() const;
    void setAdvancedStyleSheet(QtAdvancedStylesheet *styleSheet);

private:
    QAppStudio();

    Q_DECLARE_PRIVATE(QAppStudio)
    QAppStudioPrivate *d_ptr;
};

#define QU_APPSTUDIO QAppStudio::instance()

#endif // QAPPSTUDIO_H
