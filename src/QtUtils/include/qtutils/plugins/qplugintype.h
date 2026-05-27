#ifndef QPLUGINTYPE_H
#define QPLUGINTYPE_H

#include "qtutils/qutilsglobal.h"
#include <QList>

class QtPlugin;
template <class T>
class QDefinedPluginType;

class QU_API_EXPORT QPluginType
{
public:
    virtual ~QPluginType();

    inline QString name() { return m_name; }
    inline QString title() { return m_title; }
    inline QString configUiForm() { return m_configUiForm; }

    template <class T>
    inline bool isForPluginType()
    {
        return nullptr != dynamic_cast<const QDefinedPluginType<T> *>(this);
    }

    virtual bool test(QtPlugin *plugin) = 0;

    inline friend bool operator <(const QPluginType &type1, const QPluginType &type2);

protected:
    QPluginType(const QString &title, const QString &form);
    void setNativeName(const QString &name);

    QString m_title, m_name, m_configUiForm;
};

inline bool operator <(const QPluginType &type1, const QPluginType &type2)
{
    return type1.m_title.compare(type2.m_title) < 0;
}

template <class T>
class QDefinedPluginType : public QPluginType
{
    friend class QPluginManager;
public:
    inline bool test(QtPlugin *plugin) override
    {
        return nullptr != dynamic_cast<T *>(plugin);
    }

protected:
    QDefinedPluginType(const QString &title, const QString &form) : QPluginType(title, form)
    {
        setNativeName(typeid(T).name());
    }
};

#endif // QPLUGINTYPE_H
