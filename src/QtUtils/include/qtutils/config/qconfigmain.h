#ifndef QCONFIGMAIN_H
#define QCONFIGMAIN_H

#include "qtutils/qutilsglobal.h"

#include <QHash>

class QConfigEntry;
class QConfigCategory;
class QAbstractConfiguration;

class QU_API_EXPORT QConfigMain
{
public:
    QConfigMain(const QString &name, const QString &title, const char *metaName, bool presistable);
    ~QConfigMain();

    inline QString name() const { return m_name; }
    inline const char *metaName() const { return m_pMetaName; }
    inline bool isPersistable() const { return m_isPersistable; }

    void translateTitle();
    inline QString title() const { return m_title; }

    QStringList paths() const;
    QVariant toVariant() const;
    void setValues(const QVariant &var);

    void reset();
    void savePoint(bool isTransaction = false);
    inline void begin() { savePoint(true); }
    void restore();
    void release();
    inline void commit() { release(); }
    inline void rollback() { restore(); }

    QList<QConfigEntry *> entries() const;
    inline QHash<QString, QConfigCategory *> &categories() { return m_childs; }

    inline operator QConfigMain *() { return this; }

    static void init(QAbstractConfiguration *config);
    static QConfigMain *instance(const QString &name);
    static QList<QConfigMain *> instances();
    static QList<QConfigMain *> persistableInstances();
    static QConfigCategory *categoryForName(const QString &name);
    static QConfigEntry *entryForName(const QString &categoryName, const QString &name);
    static QConfigEntry *entryForFullKey(const QString &fullKey);
    static QConfigEntry *entryForPath(const QString &path);

private:
    QString m_name, m_title;
    const char *m_pMetaName;
    bool m_isPersistable;
    QHash<QString, QConfigCategory *> m_childs;

    static QList<QConfigMain *> *m_pInstance;

    friend class QConfigCategory;
};

#endif // QCONFIGMAIN_H
