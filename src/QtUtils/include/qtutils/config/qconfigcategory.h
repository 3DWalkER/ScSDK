#ifndef QCONFIGCATEGORY_H
#define QCONFIGCATEGORY_H

#include "qtutils/qutilsglobal.h"

#include <QHash>
#include <QObject>

class QConfigMain;
class QConfigEntry;

class QU_API_EXPORT QConfigCategory : public QObject
{
    Q_OBJECT
public:
    explicit QConfigCategory(const QConfigCategory &other);
    explicit QConfigCategory(const QString &name, const QString &title);

    inline QString name() const { return m_name; }
    inline QString title() const { return m_title; }
    void translateTitle();

    void reset();
    void savePoint(bool isTransaction = false);
    inline void begin() { return savePoint(true); }
    void restore();
    void release();
    inline void commit() { release(); }
    void rollback();

    QConfigEntry *entryForName(const QString &name) const;
    inline QHash<QString, QConfigEntry *> &entries() { return m_childs; }
    inline QConfigMain *main() const { return m_pParent; }

    inline operator QString() { return m_name; }
    inline operator QConfigCategory *() { return this; }

private:
    QString m_name;
    QString m_title;

    bool m_isPresistable;

    QConfigMain *m_pParent;
    QHash<QString, QConfigEntry *> m_childs;

    friend class QConfigEntry;
    friend class QConfigMain;

public slots:
    void handleEntryChanged();

signals:
    void changed(QConfigEntry *entry);
};

Q_DECLARE_METATYPE(QConfigCategory *)

#endif // QCONFIGCATEGORY_H
