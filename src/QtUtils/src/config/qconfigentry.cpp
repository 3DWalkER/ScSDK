#include "qtutils/config/qconfigentry.h"

#include "qtutils/config/qconfigcategory.h"
#include "qtutils/config/qabstractconfiguration.h"
#include "qtutils/config/qconfigmain.h"

#include <QDebug>

extern QConfigCategory *lastCreatedConfigCategory;
QAbstractConfiguration *QConfigEntry::m_pConfig = nullptr;

QConfigEntry::QConfigEntry(const QConfigEntry &other)
    : m_name(other.m_name)
    , m_defaultVal(other.m_defaultVal)
    , m_pDefaultValFunc(other.m_pDefaultValFunc)
    , m_title(other.m_title)
    , m_pParent(other.m_pParent)
    , m_isPersistable(other.m_isPersistable)
{
    connect(this, SIGNAL(changed(QVariant)), m_pParent, SLOT(handleEntryChanged()));
}

QConfigEntry::QConfigEntry(const QString &name, const QVariant &val, const QString &title)
    : m_name(name)
    , m_defaultVal(val)
    , m_pDefaultValFunc(nullptr)
    , m_title(title)
{
    if (nullptr == lastCreatedConfigCategory)
    {
        qDebug() << Q_FUNC_INFO << "No last created configuration categroy while creating QConfigEntry!";
        return;
    }

    m_pParent = lastCreatedConfigCategory;
    m_isPersistable = m_pParent->m_isPresistable;
    m_pParent->m_childs[name] = this;
    connect(this, SIGNAL(changed(QVariant)), m_pParent, SLOT(handleEntryChanged()));
}

QConfigEntry::~QConfigEntry()
{

}

QVariant QConfigEntry::get(const QString &key) const
{
    if (m_isCached)
        return m_cachedVal;

    QVariant var;
    if (m_isPersistable)
    {
        if (nullptr != m_pConfig)
            var = m_pConfig->get(key, m_name);
    }

    m_cachedVal = var;
    m_isCached = true;
    if (!m_isPersistable || !var.isValid())
    {
        m_cachedVal = nullptr != m_pDefaultValFunc ? (*m_pDefaultValFunc)() : m_defaultVal;
        return m_cachedVal;
    }

    return var;
}

void QConfigEntry::set(const QString &key, const QVariant &value)
{
    bool doPersist = m_isPersistable && !m_isTransaction;
    bool wasChanged = value != m_cachedVal;

    if (doPersist && wasChanged)
    {
        if (nullptr != m_pConfig)
            m_pConfig->set(key, m_name, value);
    }

    if (wasChanged)
    {
        m_cachedVal = value;
        emit changed(value);
    }
    m_isCached = true;

    if (doPersist)
        emit persisted(value);
}

QString QConfigEntry::fullKey() const
{
    QString res = m_pParent->m_name + "." + m_name;
    return nullptr == m_pParent->main() ? res : m_pParent->main()->name() + "." + res;
}

QString QConfigEntry::partialKey() const
{
    return m_pParent->name() + "." + m_name;
}

bool QConfigEntry::isPresisted() const
{
    if (m_isPersistable)
    {
        if (nullptr != m_pConfig)
            return !m_pConfig->get(m_pParent->m_name, m_name).isNull();
    }

    return false;
}

void QConfigEntry::savePoint(bool isTransaction)
{
    m_backup = get();
    m_isTransaction = isTransaction;
}

void QConfigEntry::restore()
{
    m_cachedVal = m_backup;
    m_isCached = true;
    m_isTransaction = false;
}

void QConfigEntry::release()
{
    m_backup.clear();
    if (!m_isTransaction)
        return;

    m_isTransaction = false;
    if (m_isCached)
    {
        QVariant var = m_cachedVal;
        m_cachedVal = QVariant();
        m_isCached = false;
        set(var);
    }
}

QConfigMain *QConfigEntry::main() const
{
    return m_pParent->m_pParent;
}
