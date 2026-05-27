#include "qtutils/plugins/qpluginmanager_p.h"

#include "qtutils/plugins/qtplugin.h"
#include "qtutils/utils/qtranslation.h"

#include <QDir>
#include <QPluginLoader>
#include <QCoreApplication>
#include <QDebug>

QPluginManagerPrivate::QPluginManagerPrivate(QPluginManager *q)
    : q_ptr(q)
{

}

void QPluginManagerPrivate::scanPlugins()
{
    QPluginLoader *loader = nullptr;
    for (const QString &path : m_pluginDirs)
    {
        QDir dir(path);
        for (QString &fileName : dir.entryList(sharedLibFileFilters(), QDir::Files))
        {
            fileName = dir.absoluteFilePath(fileName);
            loader = new QPluginLoader(fileName);
            loader->setLoadHints(QLibrary::ExportExternalSymbolsHint | QLibrary::ResolveAllSymbolsHint);

            if (!initPlugin(loader, fileName))
            {
                qDebug() << "File" << fileName << "was loaded as plugin, but SQLiteStudio couldn't initialize plugin.";
                delete loader;
            }
        }
    }
}

bool QPluginManagerPrivate::initPlugin(QPluginLoader *loader, const QString &fileName)
{
    QJsonObject metaData = loader->metaData();
    QJsonObject metaObject = metaData.value("MetaData").toObject();
    QString pluginTypeName = metaObject.value("type").toString();

    QPluginType *pluginType = nullptr;
    for (QPluginType *type : m_registeredPluginTypes)
    {
        if (type->name() == pluginTypeName)
        {
            pluginType = type;
            break;
        }
    }

    QString pluginName = metaData.value("className").toString();
    if (nullptr == pluginType)
    {
        qWarning() << QString("Could not load plugin named %1, because its type was not recognized.").arg(pluginTypeName);
        return false;
    }

    if (!checkPluginRequirements(pluginName, metaObject))
        return false;

    QPluginContainer *container = new QPluginContainer();
    container->type = pluginType;
    container->filePath = fileName;
    container->isLoaded = false;
    container->loader = loader;
    m_pluginCategories[pluginType] << container;
    m_pluginNameToContainer[pluginName] = container;

    if (!readMetaData(container))
    {
        delete container;
        return false;
    }

    return true;
}

bool QPluginManagerPrivate::initPlugin(QtPlugin *plugin)
{
    QString pluginName = plugin->name();
    QPluginType *pluginType = nullptr;
    for (QPluginType *type : m_registeredPluginTypes)
    {
        if (type->test(plugin))
        {
            pluginType = type;
            break;
        }
    }

    if (nullptr == pluginType)
    {
        qWarning() << QString("Could not load built-in plugin named %1, because its type was not recognized.").arg(pluginName);
        return false;
    }

    QPluginContainer *container = new QPluginContainer();
    container->type = pluginType;
    container->isLoaded = true;
    container->isBuiltIn = true;
    container->plugin = plugin;
    m_pluginCategories[pluginType] << container;
    m_pluginNameToContainer[pluginName] = container;
    if (!readMetaData(container))
    {
        delete container;
        return false;
    }

    handlePluginLoaded(container);

    return true;
}

bool QPluginManagerPrivate::checkPluginRequirements(const QString &pluginName, const QJsonObject &metaObject)
{
    int minVer = metaObject.value("minQtVersion").toInt(0);
    if (QT_VERSION_CHECK(minVer / 10000, minVer / 100 % 100, minVer % 10000) > QT_VERSION)
    {
        qDebug() << "Plugin" << pluginName << "skipped, because it requires at least Qt version" << QPluginManager::toPrintableVersion(minVer) << ", but got" << QT_VERSION_STR;
        return false;
    }

    int maxVer = metaObject.value("maxQtVersion").toInt(999999);
    if (QT_VERSION_CHECK(maxVer / 10000, maxVer / 100 % 100, maxVer % 10000) < QT_VERSION)
    {
        qDebug() << "Plugin" << pluginName << "skipped, because it requires at most Qt version" << QPluginManager::toPrintableVersion(maxVer) << ", but got" << QT_VERSION_STR;
        return false;
    }

    return true;
}

void QPluginManagerPrivate::handlePluginLoaded(QPluginContainer *container)
{
    if (!container->isBuiltIn)
    {
        QString tsName = container->translationName.isEmpty() ?
                    (container->name.endsWith("Plugin")
                     ? container->name.left(container->name.length() - 6)
                     : container->name)
                  : container->translationName;

        Qu::loadTranslation(tsName, "");
        container->plugin = dynamic_cast<QtPlugin *>(container->loader->instance());
        container->isLoaded = true;
    }

    emit q_func()->loaded(container->plugin, container->type);
}

bool QPluginManagerPrivate::readMetaData(QPluginContainer *container)
{
    if (nullptr != container->loader)
    {
        QHash<QString, QVariant> metaData = q_func()->readMetaData(container->loader->metaData());
        container->name = metaData["name"].toString();
        container->version = metaData["version"].toInt();
        container->printableVersion = QPluginManager::toPrintableVersion(metaData["version"].toInt());
        container->author = metaData["author"].toString();
        container->description = metaData["description"].toString();
        container->title = metaData["title"].toString();
        container->loadByDefault = metaData.contains("loadByDefault") ? metaData["loadByDefault"].toBool() : true;
        container->translationName = metaData.contains("translationName") ? metaData["translationName"].toString() : QString();
    }
    else if (nullptr != container->plugin)
    {
        container->name = container->plugin->name();
        container->version = container->plugin->version();
        container->printableVersion = container->plugin->printableVersion();
        container->author = container->plugin->author();
        container->description = container->plugin->description();
        container->title = container->plugin->title();
        container->loadByDefault = true;
    }
    else
    {
        qCritical() << "Could not read metadata for some plugin. It has no loader or plugin object defined.";
        return false;
    }

    return true;
}
