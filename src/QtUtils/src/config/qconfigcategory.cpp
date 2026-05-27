#include "qtutils/config/qconfigcategory.h"

#include "qtutils/config/qconfigmain.h"
#include "qtutils/config/qconfigentry.h"

#include <QDebug>

extern QConfigMain *lastCreatedConfigMain;
QConfigCategory *lastCreatedConfigCategory = nullptr;

QConfigCategory::QConfigCategory(const QConfigCategory &other)
    : m_name(other.m_name)
    , m_title(other.m_title)
    , m_isPresistable(other.m_isPresistable)
    , m_pParent(other.m_pParent)
    , m_childs(other.m_childs)
{
    lastCreatedConfigCategory = this;
    m_pParent->m_childs[m_name] = this;
    for (QConfigEntry *&entry : m_childs)
        entry->m_pParent = this;
}

QConfigCategory::QConfigCategory(const QString &name, const QString &title)
    : m_name(name)
    , m_title(title)
{
    lastCreatedConfigCategory = this;
    if (nullptr == lastCreatedConfigMain)
    {
        qDebug() << __FUNCTION__ << "No last created configuration main while creating QConfigCategory!";
        return;
    }

    m_pParent = lastCreatedConfigMain;
    m_pParent->m_childs[name] = this;
    m_isPresistable = m_pParent->m_isPersistable;
}

QConfigEntry *QConfigCategory::entryForName(const QString &name) const
{
    if (m_childs.contains(name))
        return m_childs[name];

    return nullptr;
}

void QConfigCategory::translateTitle()
{
    m_title = QObject::tr(m_title.toUtf8().constData());
    for (QConfigEntry *&entry : m_childs)
        entry->translateTitle();
}

void QConfigCategory::savePoint(bool isTransaction)
{
    for (QConfigEntry *&entry : m_childs)
        entry->savePoint(isTransaction);
}

void QConfigCategory::restore()
{
    for (QConfigEntry *&entry : m_childs)
        entry->restore();
}

void QConfigCategory::release()
{
    for (QConfigEntry *&entry : m_childs)
        entry->release();
}

void QConfigCategory::rollback()
{
    for (QConfigEntry *&entry : m_childs)
        entry->rollback();
}

void QConfigCategory::reset()
{
    for (QConfigEntry *&entry : m_childs)
        entry->reset();
}

void QConfigCategory::handleEntryChanged()
{
    emit changed(dynamic_cast<QConfigEntry *>(sender()));
}
