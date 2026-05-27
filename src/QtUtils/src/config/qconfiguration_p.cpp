#include "qtutils/config/qconfiguration_p.h"

#include <QDir>
#include <QSettings>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#define QCFG_DB_CONNECTION_NAME     "QConfiguration_QSQLITE"
#define QCFG_DB_DIR_SETTING         "Path/ConfigDir"
#define QCFG_DB_MEMORY_NAME         ":memory:"

QConfigurationPrivate::QConfigurationPrivate(QConfiguration *q, const QString &dbName)
    : q_ptr(q)
    , m_db(QSqlDatabase::addDatabase("QSQLITE", QCFG_DB_CONNECTION_NAME))
    , m_dbName(dbName + "_settings")
{
    initDbFile();
    initTables();
}

QConfigurationPrivate::~QConfigurationPrivate()
{
    if (m_db.isOpen()) m_db.close();
}

QVariant QConfigurationPrivate::deserialize(const QByteArray &bytes)
{
    if (bytes.isNull())
        return QVariant();

    QVariant value;
    QDataStream ss(bytes);
    ss >> value;

    return value;
}

QVariant QConfigurationPrivate::deserialize(const QVariant &value)
{
    if (!value.isValid())
        return QVariant();

    return deserialize(value.toByteArray());
}

QByteArray QConfigurationPrivate::serializeToBytes(const QVariant &value)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << value;

    return bytes;
}

void QConfigurationPrivate::initDbFile()
{
    Q_Q(QConfiguration);

    QSettings *sett = q->settings();
    QString path = sett->value(QCFG_DB_DIR_SETTING).toString();

    QList<ConfigDirCandidate> paths;
    if (!path.isEmpty())
        paths << ConfigDirCandidate{ path, true, false };
    paths << standardDbPaths();
    paths << ConfigDirCandidate{ QCFG_DB_MEMORY_NAME, false, false };

    QDir dir;
    for (ConfigDirCandidate &path : paths)
    {
        if (tryOpenDbFile(path))
        {
            dir.setPath(path.path);
            m_configDir = dir.absolutePath();
            break;
        }
    }

    if (QCFG_DB_MEMORY_NAME != m_configDir)
        return;

    while (true)
    {
        path = QFileDialog::getExistingDirectory(nullptr, QObject::tr("Select configuration directory"), QString(), QFileDialog::ShowDirsOnly);
        if (path.isEmpty())
            break;

        if (tryOpenDbFile(ConfigDirCandidate{ path + "/" + m_dbName, false, false }))
        {
            dir.setPath(path);
            m_configDir = dir.absolutePath();
            sett->setValue(QCFG_DB_DIR_SETTING, m_configDir);
            break;
        }
    }

    if (QCFG_DB_MEMORY_NAME == m_configDir)
    {
        paths.removeLast();
        QStringList pathList;
        for (const ConfigDirCandidate &path : paths)
            pathList << path.path;

        qWarning() << QString("Could not initialize configuration file: %1. "
                              "Any configuration changes and queries history will be lost after application restart.")
                      .arg(pathList.join(", "));
    }
}

void QConfigurationPrivate::initTables()
{
    QSqlQuery query(m_db);
    if (!query.exec("SELECT lower(name) AS name FROM sqlite_master WHERE type = 'table'"))
        qDebug() << __FUNCTION__ << QString("Failed to obtain table name: %1").arg(query.lastError().text());
    QStringList tables;
    while (query.next())
        tables << query.value(0).toString();

    if (!tables.contains("version"))
    {
        for (const QString &table : tables)
            m_db.exec("DROP TABLE " + table);

        tables.clear();
        m_db.exec("CREATE TABLE version (version NUMERIC)");
        m_db.exec("INSERT INTO version VALUES(" + QString::number(1) + ")");
    }

    if (!tables.contains("settings"))
        if (!m_db.exec("CREATE TABLE settings ([group] TEXT, [key] TEXT, value, PRIMARY KEY([group], [key]))").isActive())
            qDebug() << __FUNCTION__ << QString("Failed to create table named %1: %2").arg("settings").arg(m_db.lastError().text());
}

bool QConfigurationPrivate::tryOpenDbFile(const ConfigDirCandidate &candidate)
{
    if (candidate.isAutoCreate && !candidate.path.isEmpty())
    {
        QDir dir(candidate.path);
        if (!dir.exists())
        {
            if (!QDir::root().mkpath(dir.absolutePath()))
            {
                qWarning() << "Failed to make path for database read and write!";
                return false;
            }
        }
    }

    m_db.setDatabaseName(candidate.path + "/" + m_dbName);
    if (!m_db.open()) return false;

    return true;
}

QList<QConfigurationPrivate::ConfigDirCandidate> QConfigurationPrivate::standardDbPaths()
{
    QList<ConfigDirCandidate> paths;
    QString portablePath = QConfiguration::portablePath();
    if (!portablePath.isEmpty())
        paths << ConfigDirCandidate{ portablePath, false, true };

    paths << ConfigDirCandidate{ QConfiguration::appConfigPath(), true, false };

    return paths;
}
