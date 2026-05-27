#include "qtutils/config/qconfigmain.h"

#include "qtutils/config/qconfigentry.h"
#include "qtutils/config/qconfigcategory.h"
#include "qtutils/config/qabstractconfiguration.h"

#include <QDebug>

QConfigMain *lastCreatedConfigMain = nullptr;
QList<QConfigMain *> *QConfigMain::m_pInstance = nullptr;

QConfigMain::QConfigMain(const QString &name, const QString &title, const char *metaName, bool presistable)
    : m_name(name)
    , m_title(title)
    , m_pMetaName(metaName)
    , m_isPersistable(presistable)
{
    lastCreatedConfigMain = this;
    if (nullptr == m_pInstance)
        m_pInstance = new QList<QConfigMain *>();
    *m_pInstance << this;
}

QConfigMain::~QConfigMain()
{
    if (nullptr != m_pInstance)
        m_pInstance->removeOne(this);
}

void QConfigMain::translateTitle()
{
    m_title = QObject::tr(m_title.toUtf8().constData());
    for (QConfigCategory *categroy : m_childs)
        categroy->translateTitle();
}

QStringList QConfigMain::paths() const
{
    QStringList result;
    for (QConfigCategory *categroy : m_childs)
    {
        for (const QString &entryName : categroy->entries().keys())
            result << QString::fromLocal8Bit("%1.%2").arg(categroy->name()).arg(entryName);
    }

    return result;
}

QVariant QConfigMain::toVariant() const
{
    QHash<QString, QConfigEntry *> entries;
    QHash<QString, QVariant> entriesVariant;
    QHash<QString, QVariant> categoriesVariant;
    for (auto categoryIt = m_childs.begin(); categoryIt != m_childs.end(); categoryIt++)
    {
        entries = categoryIt.value()->entries();
        entriesVariant.clear();
        for (auto entryIt = entries.begin(); entryIt != entries.end(); entryIt++)
            entriesVariant[entryIt.key()] = entryIt.value()->get();

        categoriesVariant[categoryIt.key()] = entriesVariant;
    }

    QHash<QString, QVariant> mainVariant;
    mainVariant[m_name] = categoriesVariant;
    return mainVariant;
}

void QConfigMain::setValues(const QVariant &var)
{
    QHash<QString, QVariant> mainVariant = var.toHash();
    if (mainVariant.isEmpty())
        return;

    auto mainIt = mainVariant.begin();
    if (mainIt.key() != m_name)
        return;

    QVariant entryVariant;
    QHash<QString, QConfigEntry *> entries;
    QHash<QString, QVariant> entriesVariant;
    QHash<QString, QVariant> categoriesVariant = mainIt.value().toHash();
    for (auto categoryIt = m_childs.begin(); categoryIt != m_childs.end(); categoryIt++)
    {
        entriesVariant = categoriesVariant.value(categoryIt.key()).toHash();
        if (entriesVariant.isEmpty())
            continue;

        entries = categoryIt.value()->entries();
        for (auto entryIt = entries.begin(); entryIt != entries.end(); entryIt++)
        {
            entryVariant = entriesVariant.value(entryIt.key());
            if (entryVariant.isValid())
                entryIt.value()->set(entryVariant);
        }
    }
}

void QConfigMain::reset()
{
    for (QConfigCategory *categroy : m_childs)
        categroy->reset();
}

void QConfigMain::savePoint(bool isTransaction)
{
    for (QConfigCategory *categroy : m_childs)
        categroy->savePoint(isTransaction);
}

void QConfigMain::restore()
{
    for (QConfigCategory *categroy : m_childs)
        categroy->restore();
}

void QConfigMain::release()
{
    for (QConfigCategory *categroy : m_childs)
        categroy->release();
}

QList<QConfigEntry *> QConfigMain::entries() const
{
    QList<QConfigEntry *> result;
    for (QConfigCategory *categroy : m_childs)
        result += categroy->entries().values();

    return result;
}

void QConfigMain::init(QAbstractConfiguration *config)
{
    if (nullptr != QConfigEntry::m_pConfig)
        delete QConfigEntry::m_pConfig;
    QConfigEntry::m_pConfig = config;

    qRegisterMetaType<QConfigEntry *>("QConfigEntry");
    qRegisterMetaType<QConfigCategory *>("QConfigCategory");
    qRegisterMetaType<QConfigMain *>("QConfigMain");
}

QConfigMain *QConfigMain::instance(const QString &name)
{
    if (nullptr == m_pInstance)
        return nullptr;

    for (QConfigMain *main : *m_pInstance)
    {
        if (main->name() == name)
            return main;
    }
    return nullptr;
}

QList<QConfigMain *> QConfigMain::instances()
{
    if (nullptr == m_pInstance)
        m_pInstance = new QList<QConfigMain *>();

    return *m_pInstance;
}

QList<QConfigMain *> QConfigMain::persistableInstances()
{
    QList<QConfigMain *> result;
    if (nullptr != m_pInstance)
    {
        for (QConfigMain *main : *m_pInstance)
        {
            if (main->isPersistable())
                result << main;
        }
    }

    return result;
}

QConfigCategory *QConfigMain::categoryForName(const QString &name)
{
    for (QConfigMain *main : instances())
    {
        if (main->m_childs.contains(name))
            return main->m_childs[name];
    }

    return nullptr;
}

QConfigEntry *QConfigMain::entryForName(const QString &categoryName, const QString &name)
{
    QConfigCategory *category = categoryForName(categoryName);
    if (nullptr == category)
        return nullptr;

    return category->entryForName(name);
}

QConfigEntry *QConfigMain::entryForFullKey(const QString &fullKey)
{
    QStringList list = fullKey.split(".");
    if (3 != list.size())
    {
        qWarning() << __FUNCTION__ << QString::fromLocal8Bit("The full key format '%1' of entry is error!").arg(fullKey);
        return nullptr;
    }

    QConfigMain *pMain = instance(list[0]);
    if (nullptr == pMain)
        return nullptr;

    QConfigCategory *pCategory = pMain->m_childs.value(list[1]);
    if (nullptr == pCategory)
        return nullptr;

    return nullptr == pCategory ? nullptr : pCategory->m_childs.value(list[2]);
}

QConfigEntry *QConfigMain::entryForPath(const QString &path)
{
    QStringList list = path.split(".");
    if (2 != list.size())
        return nullptr;

    return entryForName(list[0], list[1]);
}
