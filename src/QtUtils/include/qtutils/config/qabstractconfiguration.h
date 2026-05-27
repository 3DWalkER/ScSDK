#ifndef QABSTRACTCONFIGURATION_H
#define QABSTRACTCONFIGURATION_H

#include "qtutils/qutilsglobal.h"
#include <QObject>

class QSettings;

class QU_API_EXPORT QAbstractConfiguration : public QObject
{
    Q_OBJECT
public:
    virtual ~QAbstractConfiguration();

    virtual void beginMassSave() = 0;
    virtual void commitMassSave() = 0;
    virtual void rollbackMassSave() = 0;

    virtual QHash<QString, QVariant> get(const QString &group) = 0;
    virtual QVariant get(const QString &group, const QString &key) = 0;
    virtual QVariant get(const QString &group, const QString &key, const QVariant &defaultValue) = 0;
    virtual QHash<QString, QVariant> all() = 0;
    virtual void set(const QString &group, const QString &key, const QVariant &value) = 0;
    virtual void remove(const QString &group, const QString &key) = 0;

    virtual bool addDatabase(const QString &name, const QString &path, const QHash<QString, QVariant> &options) = 0;

    inline bool isMassSaving() { return m_isMassSaving; }

    static QString portablePath();
    static QString appConfigPath();
    static QSettings *settings();

protected:
    bool m_isMassSaving = false;

signals:
    void massSaveBegins();
    void massSaveCommitted();
};

#define QU_CONFIG QU_APPSTUDIO->configuration()

#endif // QABSTRACTCONFIGURATION_H
