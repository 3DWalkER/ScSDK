#ifndef QCONFIGURATION_H
#define QCONFIGURATION_H

#include "qabstractconfiguration.h"
#include <QSqlDatabase>

class QConfigurationPrivate;

class QU_API_EXPORT QConfiguration : public QAbstractConfiguration
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QConfiguration)
public:
    explicit QConfiguration(const QString &dbName);
    virtual ~QConfiguration();

    void beginMassSave() override;
    void commitMassSave() override;
    void rollbackMassSave() override;

    QHash<QString, QVariant> get(const QString &group) override;
    QVariant get(const QString &group, const QString &key) override;
    QVariant get(const QString &group, const QString &key, const QVariant &defaultValue) override;
    QHash<QString, QVariant> all() override;
    void set(const QString &group, const QString &key, const QVariant &value) override;
    void remove(const QString &group, const QString &key) override;

    bool addDatabase(const QString &name, const QString &path, const QHash<QString, QVariant> &options) override;

private:
    QConfigurationPrivate *d_ptr;
};

#endif // QCONFIGURATION_H
