#ifndef QCONFIGENTRY_H
#define QCONFIGENTRY_H

#include "qtutils/config/qconfigcategory.h"

class QAbstractConfiguration;

class QU_API_EXPORT QConfigEntry : public QObject
{
    Q_OBJECT
public:
    typedef QVariant (*DefaultValueProvidorFunc)();

    explicit QConfigEntry(const QConfigEntry &other);
    QConfigEntry(const QString &name, const QVariant &val, const QString &title);
    virtual ~QConfigEntry();

    QVariant get(const QString &key) const;
    inline QVariant get() const { return get(m_pParent->m_name); }
    inline QVariant defaultValue() const { return nullptr != m_pDefaultValFunc ? (*m_pDefaultValFunc)() : m_defaultVal; }
    void set(const QString &key, const QVariant &value);
    inline void set(const QVariant &value) { return set(m_pParent->m_name, value); }
    inline void reset() { set(defaultValue()); }
    inline void setDefaultValueFunction(DefaultValueProvidorFunc func) { m_pDefaultValFunc = func; }

    inline QString name() const { return m_name; }
    QString fullKey() const;
    QString partialKey() const;
    inline QString title() const { return m_title; }
    inline void translateTitle() { m_title = QObject::tr(m_title.toUtf8().constData()); }

    inline bool isPersistable() const { return m_isPersistable; }
    bool isPresisted() const;

    void savePoint(bool isTransaction = false);
    inline void begin() { return savePoint(true); }
    void restore();
    void release();
    inline void commit() { if (m_isTransaction) release(); }
    inline void rollback() { if (m_isTransaction) restore(); }

    QConfigMain *main() const;
    inline QConfigCategory *category() const { return m_pParent; }

    inline operator QString() { return m_name; }
    inline operator QConfigEntry *() { return this; }

protected:
    QString m_name, m_title;
    QVariant m_defaultVal, m_backup;
    DefaultValueProvidorFunc m_pDefaultValFunc;

    QConfigCategory *m_pParent;

    bool m_isPersistable;
    bool m_isTransaction = false;

    mutable QVariant m_cachedVal;
    mutable bool m_isCached = false;

    static QAbstractConfiguration *m_pConfig;

    friend class QConfigMain;
    friend class QConfigCategory;

signals:
    void changed(const QVariant &newVal);
    void persisted(const QVariant &newVal);
};

template <class T>
class QConfigTypedEntry : public QConfigEntry
{
public:
    QConfigTypedEntry(const QString &name, DefaultValueProvidorFunc func, const QString &title)
        : QConfigEntry(name, QVariant(), title) { setDefaultValueFunction(func); }

    QConfigTypedEntry(const QString &name, const T &value, const QString &title)
        : QConfigEntry(name, QVariant::fromValue(value), title) { }

    QConfigTypedEntry(const QString &name, DefaultValueProvidorFunc func)
        : QConfigTypedEntry(name, func, QString()) { }

    QConfigTypedEntry(const QString &name, DefaultValueProvidorFunc func, bool isPersistable)
        : QConfigTypedEntry(name, func, QString()) { m_isPersistable = isPersistable; }

    QConfigTypedEntry(const QString &name, const T &value, bool isPersistable)
        : QConfigTypedEntry(name, value, QString()) { m_isPersistable = isPersistable; }

    QConfigTypedEntry(const QString &name,  const T &value)
        : QConfigTypedEntry(name, value, QString()) { }

    QConfigTypedEntry(const QString &name)
        : QConfigEntry(name, QVariant(), QString()) { }

    QConfigTypedEntry(const QConfigTypedEntry &other)
        : QConfigEntry(other) { }

    T get() const
    {
        QVariant val = QConfigEntry::get();

        return val.value<T>();
    }

    void set(const T &value)
    {
        QConfigEntry::set(QVariant::fromValue<T>(value));
    }
};

Q_DECLARE_METATYPE(QConfigEntry *)

#endif // QCONFIGENTRY_H
