#include "qappstudio.h"
#include "qabstractconfiguration.h"
#include "qpluginmanager.h"
#include "qtadvancedstylesheet.h"
#include "qsqldatabasemanager.h"

Q_DEFINE_SIGNLETON_HUNGRY(QAppStudio)

class QAppStudioPrivate
{
    Q_DECLARE_PUBLIC(QAppStudio)
public:
    QAppStudioPrivate(QAppStudio *q) : q_ptr(q) { }

    /**
     * @brief q_ptr q指针
     */
    QAppStudio *q_ptr;

    /**
     * @brief m_pConfig 程序配置
     */
    QAbstractConfiguration *m_pConfig = nullptr;

    /**
     * @brief m_pPluginMgr 插件管理器
     */
    QPluginManager *m_pPluginMgr = nullptr;

    /**
     * @brief m_pPluginMgr 数据库管理器
     */
    QSqlDatabaseManager *m_pDatabaseMgr { };

    /**
     * @brief m_pStyleSheet 样式表
     */
    QtAdvancedStylesheet *m_pStyleSheet = nullptr;
};


QAppStudio::QAppStudio()
    : d_ptr(new QAppStudioPrivate(this))
{

}

QAppStudio::~QAppStudio()
{

}

QAbstractConfiguration *QAppStudio::configuration() const
{
    Q_D(const QAppStudio);
    return d->m_pConfig;
}

void QAppStudio::setConfiguration(QAbstractConfiguration *config)
{
    Q_D(QAppStudio);
    QU_SAVE_DELETE(d->m_pConfig);
    d->m_pConfig = config;
}

QPluginManager *QAppStudio::pluginManager() const
{
    Q_D(const QAppStudio);
    return d->m_pPluginMgr;
}

void QAppStudio::setePluginManager(QPluginManager *mgr)
{
    Q_D(QAppStudio);
    QU_SAVE_DELETE(d->m_pPluginMgr);
    d->m_pPluginMgr = mgr;
}

QSqlDatabaseManager *QAppStudio::databaseManager() const
{
    Q_D(const QAppStudio);
    return d->m_pDatabaseMgr;
}

void QAppStudio::setDatabaseManager(QSqlDatabaseManager *mgr)
{
    Q_D(QAppStudio);
    QU_SAVE_DELETE(d->m_pDatabaseMgr);
    d->m_pDatabaseMgr = mgr;
}

QtAdvancedStylesheet *QAppStudio::advancedStylesheet() const
{
    Q_D(const QAppStudio);
    return d->m_pStyleSheet;
}

void QAppStudio::setAdvancedStyleSheet(QtAdvancedStylesheet *styleSheet)
{
    Q_D(QAppStudio);
    QU_SAVE_DELETE(d->m_pStyleSheet);
    d->m_pStyleSheet = styleSheet;
}
