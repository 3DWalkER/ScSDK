#ifndef QCONFIGURATIONPRIVATE_H
#define QCONFIGURATIONPRIVATE_H

#include "qconfiguration.h"

class QConfigurationPrivate
{
    Q_DECLARE_PUBLIC(QConfiguration)
public:
    QConfigurationPrivate(QConfiguration *q, const QString &dbName);
    ~QConfigurationPrivate();

    struct ConfigDirCandidate
    {
        QString path;
        bool isAutoCreate;
        bool isPortable;
    };

    QVariant deserialize(const QByteArray &bytes);
    QVariant deserialize(const QVariant &value);
    QByteArray serializeToBytes(const QVariant &value);

    void initDbFile();
    void initTables();
    bool tryOpenDbFile(const ConfigDirCandidate &candidate);
    QList<ConfigDirCandidate> standardDbPaths();

    QConfiguration *q_ptr;

    QSqlDatabase m_db;
    QString m_configDir, m_dbName;
};

#endif // QCONFIGURATIONPRIVATE_H
