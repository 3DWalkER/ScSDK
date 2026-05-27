#ifndef QPLUGINMANAGER_H
#define QPLUGINMANAGER_H

#include <QHash>
#include <QObject>
#include "qplugintype.h"

class QtPlugin;
class QPluginManagerPrivate;

class QU_API_EXPORT QPluginManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QPluginManager)
public:
    QPluginManager();
    virtual ~QPluginManager();

    bool loadBuiltInPlugin(QtPlugin *plugin);
    QList<QtPlugin *> loadedPlugins(QPluginType *type) const;
    QList<QPluginType *> pluginTypes() const;

    QHash<QString, QVariant> readMetaData(const QJsonObject &metaData);

    template <class T>
    inline void registerPluginType(const QString &title, const QString &form = QString())
    { registerPluginType(new QDefinedPluginType<T>(title, form)); }

    template <class T>
    QPluginType *pluginType() const;

    template <class T>
    QList<T *> loadedPlugins() const;

    inline static QString toPrintableVersion(int version);

private:
    QPluginManagerPrivate *d_ptr;

protected:
    inline void registerPluginType(QPluginType *type);

signals:
    void loaded(QtPlugin *plugin, QPluginType *type);
};

QString QPluginManager::toPrintableVersion(int version)
{
    static const QString versionStr = QStringLiteral("%1.%2.%3");
    return versionStr.arg(version / 10000).arg(version / 100 % 100).arg(version % 100);
}

template<class T>
QPluginType *QPluginManager::pluginType() const
{
    for (QPluginType *type : pluginTypes())
    {
        if (nullptr == dynamic_cast<QDefinedPluginType<T> *>(type))
            continue;
        return type;
    }
    return nullptr;
}

template<class T>
QList<T *> QPluginManager::loadedPlugins() const
{
    QList<T *> result;
    QPluginType *type = pluginType<T>();
    if (nullptr == type)
        return result;

    for (QtPlugin *plugin : loadedPlugins(type))
        result << dynamic_cast<T *>(plugin);
    return result;
}

#define QU_PLUGINS QU_APPSTUDIO->pluginManager()

#endif // QPLUGINMANAGER_H
