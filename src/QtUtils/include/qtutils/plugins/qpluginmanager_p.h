#ifndef QPLUGINMANAGERPRIVATE_H
#define QPLUGINMANAGERPRIVATE_H

#include "qpluginmanager.h"

class QPluginLoader;

class QPluginManagerPrivate
{
    Q_DECLARE_PUBLIC(QPluginManager)
public:
    explicit QPluginManagerPrivate(QPluginManager *q);

    struct QPluginDependency
    {
        QString name;
        int minVersion = 0;
        int maxVersion = 0;
    };

    /**
     * @brief Container for plugin related data.
     *
     * The container is used to represent plugin available to the application,
     * no matter if it's loaded or not. It keeps all plugin related data,
     * so it's available even the plugin is not loaded.
     */
    struct QPluginContainer
    {
        /**
         * @brief Name of the plugin.
         */
        QString name;

        /**
         * @brief Title of the plugin, used on UI.
         */
        QString title;

        /**
         * @brief Plugin's detailed description.
         */
        QString description;

        /**
         * @brief Plugin's author.
         */
        QString author;

        /**
         * @brief Numeric verion of the plugin.
         */
        int version;

        /**
         * @brief Human-readable version.
         */
        QString printableVersion;

        /**
         * @brief Type of the plugin.
         */
        QPluginType *type = nullptr;

        /**
         * @brief Full path to the plugin's file.
         */
        QString filePath;

        /**
         * @brief Plugin's loaded state flag.
         */
        bool isLoaded;

        /**
         * @brief Qt's plugin framework loaded for this plugin.
         */
        QPluginLoader *loader = nullptr;

        /**
         * @brief Plugin object.
         *
         * It's null when plugin is not loaded.
         */
        QtPlugin *plugin = nullptr;

        /**
         * @brief Flag indicating that the plugin is built in.
         *
         * Plugins built-in are classes implementing plugin's interface,
         * but they are compiled and statically linked to the main application binary.
         * They cannot be loaded or unloaded - they are loaded by default.
         */
        bool isBuiltIn = false;

        /**
         * @brief Flag indicating that plugin should be loaded, unless user unloaded it manually.
         *
         * If this flag is set to false, then the plugin will not be loaded, even it was not manually unloaded.
         * This flag can be defined in plugin's json file using property named 'loadByDefault'.
         */
        bool loadByDefault = true;

        /**
         * @brief Names of plugnis that this plugin depends on.
         */
        QList<QPluginDependency> dependencies;

        /**
         * @brief Names of plugins that this plugin conflicts with.
         */
        QStringList conflicts;

        /**
         * @brief If not empty, contains Plugin's project name to be used for loading translation resource file.
         *
         * For typical SQLiteStudio plugin the auto-generated translation resource name is the same
         * as the name of the plugin project. Typically, name of loaded plugin class is made of
         * the name of the plugin project and the "Plugin" word suffix. Therefore SQLiteStudio
         * by default just removes the "Plugin" suffix (if it has such) and attempts to load the translation
         * named this way.
         *
         * If the main Plugin class does not follow this naming strategy (project name + Plugin suffix),
         * then the translationName should be specified in plugin's metadata,
         * giving actual name of translation resource (i.e. name of Plugin's source code project) to be loaded.
         */
        QString translationName;
    };

    /**
     * @brief Scans plugin directories to find out available plugins.
     *
     * It looks in the following locations:
     * <ul>
     * <li> application_directory/plugins/
     * <li> application_config_directory/plugins/
     * <li> directory pointed by the SQLITESTUDIO_PLUGINS environment variable
     * <li> directory compiled in as PLUGINS_DIR parameter of the compilation
     * </ul>
     *
     * The application_directory is a directory where the application executable is.
     * The application_config_directory can be different, see ConfigImpl::initDbFile() for details.
     * The SQLITESTUDIO_PLUGINS variable can contain several paths, separated by : (for Unix/Mac) or ; (for Windows).
     */
    void scanPlugins();

    /**
     * @brief Creates plugin container and initializes it.
     * @param loader Qt's plugin framework loader used to load this plugin.
     *               For built-in plugins (statically linked) this must be null.
     * @param fileName Plugin's file path. For built-in plugins it's ignored.
     * @param plugin Plugin object from loaded plugin.
     * @return true if the initialization succeeded, or false otherwise.
     *
     * It assigns plugin type to the plugin, creates plugin container and fills
     * all necessary data for the plugin. If the plugin was configured to not load,
     * then this method unloads the file, before plugin was initialized (with Plugin::init()).
     *
     * All plugins are loaded at the start, but before they are fully initialized
     * and enabled, they are simply queried for metadata, then either unloaded
     * (when configured to not load at startup), or the initialization proceeds.
     */
    bool initPlugin(QPluginLoader* loader, const QString& fileName);

    /**
     * @brief Creates plugin container and initializes it.
     * @param plugin Built-in plugin object.
     * @return true if the initialization succeeded, or false otherwise.
     *
     * This is pretty much the same as the other initPlugin() method, but this one is for built-in plugins.
     */
    bool initPlugin(QtPlugin *plugin);

    bool checkPluginRequirements(const QString &pluginName, const QJsonObject &metaObject);

    /**
     * @brief Executes standard routines after plugin was loaded.
     * @param container Container for the loaded plugin.
     *
     * It fills all members of the plugin container and emits loaded() signal.
     */
    void handlePluginLoaded(QPluginContainer *container);

    /**
     * @brief Reads title, description, author, etc. from the plugin.
     * @param plugin Plugin to read data from.
     * @param container Container to put the data to.
     * @return true on success, false on problems (with details in logs)
     *
     * It does the reading by calling all related methods from Plugin interface,
     * then stores those information in given \p container.
     *
     * The built-in plugins define those methods using their class metadata.
     *
     * External plugins provide this information in their file metadata
     * and this method uses QPluginLoader to read this metadata.
     */
    bool readMetaData(QPluginContainer *container);

    inline QStringList sharedLibFileFilters();

    /**
     * @brief List of plugin directories (not necessarily absolute paths).
     */
    QStringList m_pluginDirs;

    /**
     * @brief List of plugins, both loaded and unloaded.
     */
    typedef QList<QPluginContainer *> QPluginContainers;

    /**
     * @brief List of registered plugin types.
     */
    QList<QPluginType *> m_registeredPluginTypes;

    /**
     * @brief Table with plugin types as keys and list of plugins assigned for each type.
     */
    QHash<QPluginType *, QPluginContainers> m_pluginCategories;

    /**
     * @brief Table with plugin names and containers assigned for each plugin.
     */
    QHash<QString, QPluginContainer *> m_pluginNameToContainer;

    QPluginManager *q_ptr;
};

QStringList QPluginManagerPrivate::sharedLibFileFilters()
{
    return
    {
#ifdef Q_OS_WIN
        "*.dll"
#elif defined Q_OS_MACOS
        "*.dylib"
#elif defined Q_OS_LINUX || Q_OS_BSD
        "*.so"
#endif
    };
}

#endif // QPLUGINMANAGERPRIVATE_H
