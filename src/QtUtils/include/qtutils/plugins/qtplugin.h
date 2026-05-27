#ifndef QUPLUGIN_H
#define QUPLUGIN_H

#include <QObject>

class QtPlugin
{
public:
    virtual ~QtPlugin() { }

    /**
     * @brief Gets name of the plugin.
     * @return Name of the plugin.
     *
     * The name of the plugin is a kind of primary key for plugins. It has to be unique across all loaded plugins.
     * An attempt to load two plugins with the same name will result in failed load of the second plugin.
     *
     * The name is a kind of internal plugin's name. It's designated for presenting to the user
     * - for that purpose there is a title().
     *
     * It's a good practice to keep it as single word. Providing plugin's class name can be a good idea.
     */
    virtual QString name() const = 0;

    /**
     * @brief Gets title for the plugin.
     * @return Plugin title.
     *
     * This is plugin's name to be presented to the user. It can be multiple words name. It should be localized (translatable) text.
     * It's used solely for presenting plugin to the user, nothing more.
     */
    virtual QString title() const = 0;

    /**
     * @brief Provides name of the plugin's author.
     * @return Author name.
     *
     * This is displayed in ConfigDialog when user clicks on Details button of the plugin.
     */
    virtual QString author() const = 0;

    /**
     * @brief Provides some details on what does the plugin.
     * @return Plugin description.
     *
     * This is displayed in ConfigDialog when user clicks on Details button of the plugin.
     */
    virtual QString description() const = 0;

    /**
     * @brief Provides plugin version number.
     * @return Version number.
     *
     * Version number format can be picked by plugin developer, but it is recommended
     * to use XXYYZZ, where XX is major version, YY is minor version and ZZ is patch version.
     * Of course the XX can be single X if major version is less then 10.
     *
     * This would result in versions like: 10000 (for version 1.0.0), or 10102 (for version 1.1.2),
     * or 123200 (for version 12.32.0).
     *
     * This is of course just a suggestion, you don't have to stick to it. Just keep in mind,
     * that this number is used by SQLiteStudio to compare plugin versions. If there's a plugin with higher version,
     * SQLiteStudio will propose to update it.
     *
     * The suggested format is also easier to convert to printable (string) version later in printableVersion().
     */
    virtual int version() const = 0;

    /**
     * @brief Provides formatted version string.
     * @return Version string.
     *
     * It provides string that represents version returned from getVersion() in a human-readable form.
     * It's a good practice to return versions like "1.3.2", or "1.5", as they are easy to read.
     *
     * This version string is presented to the user.
     */
    virtual inline QString printableVersion() const;

    /**
     * @brief Initializes plugin just after it was loaded.
     * @return true on success, or false otherwise.
     *
     * This is called as a first, just after plugin was loaded. If it returns false,
     * then plugin loading is considered to be failed and gets unloaded.
     *
     * If this method returns false, then deinit() is not called.
     */
    virtual bool init() = 0;

    /**
     * @brief Deinitializes plugin that is about to be unloaded.
     *
     * This is called just before plugin is unloaded. It's called only when plugin was loaded
     * successfully. It's NOT called when init() returned false.
     */
    virtual void deinit() = 0;

protected:
    /**
     * @brief Extracts class meta information with given key.
     * @param key Key to extract.
     * @return Value of the meta information, or null if there's no information with given key.
     *
     * This is a helper method which queries Qt's meta object subsystem for class meta information defined with Q_CLASSINFO.
     */
    const char *metaInfo(const QString& key) const;
};

inline QString QtPlugin::printableVersion() const
{
    int version = this->version();
    QString versionFormat = QStringLiteral("%1.%2.%3");

    return versionFormat.arg(version / 10000) .arg(version / 100 % 100).arg(version % 100);
}

/**
 * @def QU_PLUGIN_INTERFACE
 * @brief Custom plugin interface ID.
 *
 * This is an ID string for Qt's plugins framework. It's used by ::Q_CUSTOM_PLUGIN macro.
 * No need to use it directly.
 */
#define QU_PLUGIN_INTERFACE "pl.custom.Plugin/1.0"

/**
 * @def Q_CUSTOM_PLUGIN
 * @brief Defines class as a SQLiteStudio plugin
 *
 * Every class implementing SQLiteStudio plugin must have this declaration,
 * otherwise SQLiteStudio won't be able to load the plugin.
 *
 * It has to be placed in class declaration:
 * @code
 * class MyPlugin : public QObject, public QPlugin
 * {
 *     Q_OBJECT
 *     Q_CUSTOM_PLUGIN
 *
 *     public:
 *         // ...
 * }
 * @endcode
 */
#define Q_CUSTOM_PLUGIN(file)\
    Q_PLUGIN_METADATA(IID QU_PLUGIN_INTERFACE FILE file) \
    Q_INTERFACES(QCstPlugin)

Q_DECLARE_INTERFACE(QtPlugin, QU_PLUGIN_INTERFACE)

#endif // QUPLUGIN_H
