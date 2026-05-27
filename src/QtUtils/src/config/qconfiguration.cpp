#include "qtutils/config/qconfiguration_p.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

QConfiguration::QConfiguration(const QString &dbName)
    : d_ptr(new QConfigurationPrivate(this, dbName))
{

}

QConfiguration::~QConfiguration()
{
    delete d_ptr;
}

void QConfiguration::beginMassSave()
{
    if (isMassSaving())
        return;

    Q_D(QConfiguration);

    QSqlQuery query(d->m_db);
    if (query.exec("BEGIN;"))
    {
        emit massSaveBegins();
        m_isMassSaving = true;
    }
}

void QConfiguration::commitMassSave()
{
    if (!isMassSaving())
        return;

    Q_D(QConfiguration);

    QSqlQuery query(d->m_db);
    if (query.exec("COMMIT;"))
    {
        emit massSaveCommitted();
        m_isMassSaving = false;
    }
}

void QConfiguration::rollbackMassSave()
{
    if (!isMassSaving())
        return;

    Q_D(QConfiguration);

    QSqlQuery query(d->m_db);
    if (query.exec("ROLLBACK;"))
        m_isMassSaving = false;
}

QHash<QString, QVariant> QConfiguration::get(const QString &group)
{
    Q_D(QConfiguration);
    QHash<QString, QVariant> result;
    QSqlQuery query(d->m_db);
    if (!query.exec((QString::fromLocal8Bit("SELECT * FROM settings WHERE [group] = '%1'").arg(group))))
    {
        qWarning() << __FUNCTION__ << QString("Failed to obtain all configuration values: %1").arg(query.lastError().text());
        return result;
    }

    QString key;
    while (query.next())
    {
        key = query.value("key").toString();
        result[key] = d->deserialize(query.value("value"));
    }

    return result;
}

QVariant QConfiguration::get(const QString &group, const QString &key)
{
    Q_D(QConfiguration);

    QSqlQuery query(d->m_db);
    if (query.exec(QString::fromLocal8Bit("SELECT VALUE FROM settings WHERE [group] = '%1' AND [key] = '%2'").arg(group).arg(key)))
    {
        if (query.next())
            return d->deserialize(query.value(0));
    }
    else
        qDebug() << __FUNCTION__ << QString("Failed to obtain the value corresponding to %1 in the %2: %3").arg(key).arg(group).arg(query.lastError().text());

    return QVariant();
}

QVariant QConfiguration::get(const QString &group, const QString &key, const QVariant &defaultValue)
{
    QVariant value = get(group, key);
    if (!value.isValid() || value.isNull())
        return defaultValue;

    return value;
}

QHash<QString, QVariant> QConfiguration::all()
{
    Q_D(QConfiguration);

    QHash<QString, QVariant> result;
    QSqlQuery query(d->m_db);
    if (!query.exec("SELECT * FROM settings"))
    {
        qWarning() << __FUNCTION__ << QString("Failed to obtain all configuration values: %1").arg(query.lastError().text());
        return result;
    }

    QString key;
    while (query.next())
    {
        key = query.value("group").toString() + "." + query.value("key").toString();
        result[key] = d->deserialize(query.value("value"));
    }

    return result;
}

void QConfiguration::set(const QString &group, const QString &key, const QVariant &value)
{
    Q_D(QConfiguration);
    QSqlQuery query(d->m_db);
    query.prepare(QString("INSERT OR REPLACE INTO settings VALUES('%1', '%2', :value)").arg(group).arg(key));
    query.bindValue(":value", d->serializeToBytes(value));
    if (!query.exec())
        qDebug() << __FUNCTION__ << QString("Failed to insert or replace value where group = '%1', key = '%2': %3").arg(group).arg(key).arg(query.lastError().text());
}

void QConfiguration::remove(const QString &group, const QString &key)
{
    Q_D(QConfiguration);
    QSqlQuery query(d->m_db);
    query.prepare(QString("DELETE FROM settings WHERE [group] = '%1' AND [key] = '%2'").arg(group).arg(key));
    if (!query.exec())
        qDebug() << __FUNCTION__ << QString("Failed to insert or remove value where group = '%1', key = '%2': %3").arg(group).arg(key).arg(query.lastError().text());
}

bool QConfiguration::addDatabase(const QString &name, const QString &path, const QHash<QString, QVariant> &options)
{
    return true;
}
