#include "qtutils/plugins/qpluginmanager_p.h"

#include "qtutils/plugins/qtplugin.h"

#include <QJsonObject>

QPluginManager::QPluginManager()
    : d_ptr(new QPluginManagerPrivate(this))
{

}

QPluginManager::~QPluginManager()
{
    delete d_ptr;
}

bool QPluginManager::loadBuiltInPlugin(QtPlugin *plugin)
{
    Q_D(QPluginManager);
    return d->initPlugin(plugin) && plugin->init();
}

QList<QtPlugin *> QPluginManager::loadedPlugins(QPluginType *type) const
{
    Q_D(const QPluginManager);

    QList<QtPlugin *> result;
    if (!d->m_pluginCategories.contains(type))
        return result;

    for (QPluginManagerPrivate::QPluginContainer *container : d->m_pluginCategories[type])
    {
        if (container->isLoaded)
            result << container->plugin;
    }

    return result;
}

QList<QPluginType *> QPluginManager::pluginTypes() const
{
    Q_D(const QPluginManager);
    return d->m_registeredPluginTypes;
}

QHash<QString, QVariant> QPluginManager::readMetaData(const QJsonObject &metaData)
{
    QHash<QString, QVariant> results;
    results["name"] = metaData.value("className").toString();

    QJsonObject root = metaData.value("MetaData").toObject();
    for (const QString &k : root.keys())
        results[k] = root.value(k).toVariant();

    return results;
}

void QPluginManager::registerPluginType(QPluginType *type)
{
    Q_D(QPluginManager);
    d->m_registeredPluginTypes << type;
}
