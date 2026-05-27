#ifndef QBUILTINPLUGIN_H
#define QBUILTINPLUGIN_H

#include "qtplugin.h"
#include "qtutils/qutilsglobal.h"

/**
 * @brief Helper class for implementing built-in plugins
 *
 * This class can be inherited, so most of the abstract methods from Plugin interface get implemented.
 * All details (description, name, title, author, ...) are defined using Q_CLASSINFO.
 *
 * There are macros defined to help you with defining those details. You don't need to define
 * Q_CLASSINFO and know all required keys. You can use following macros:
 * <ul>
 * <li>::SQLITESTUDIO_PLUGIN_TITLE</li>
 * <li>::SQLITESTUDIO_PLUGIN_DESC</li>
 * <li>::SQLITESTUDIO_PLUGIN_UI</li>
 * <li>::SQLITESTUDIO_PLUGIN_VERSION</li>
 * <li>::SQLITESTUDIO_PLUGIN_AUTHOR</li>
 * </ul>
 *
 * Most of plugin implementations will use this class as a base, because it simplifies process
 * of plugin development. Using this class you don't have to implement any of virtual methods
 * from Plugin interface. It's enough to define meta information, like this:
 * @code
 * class MyPlugin : QBuiltInPlugin
 * {
 *     Q_OBJECT
 *     Q_BUILD_IN_PLUGIN(Title, Desc, FormName, Version, Author)
 * };
 * @endcode
 */

class QU_API_EXPORT QBuiltInPlugin : public QObject, public virtual QtPlugin
{
    Q_INTERFACES(QCstPlugin)
public:
    /**
     * @brief Provides plugin internal name.
     * @return Plugin class name.
     */
    inline QString name() const override { return metaObject()->className(); }

    /**
     * @brief Provides plugin title.
     * @return Title defined in plugin's metadata file with key "title" or (if not defined) the same value as name().
     */
    inline QString title() const override;

    /**
     * @brief Provides an author name.
     * @return Author name as defined with in plugin's metadata file with key "author", or null QString if not defined.
     */
    inline QString author() const override { return metaInfo("author"); }

    /**
     * @brief Provides plugin description.
     * @return Description as defined in plugin's metadata file with key "description", or null QString if not defined.
     */
    inline QString description() const override { return metaInfo("description"); }

    /**
     * @brief Provides plugin numeric version.
     * @return Version number as defined in plugin's metadata file with key "version", or 0 if not defined.
     */
    inline int version() const override { return QString(metaInfo("description")).toInt(); }

    /**
     * @brief Does nothing.
     * @return Always true.
     *
     * This is a default (empty) implementation of init() for plugins.
     */
    inline bool init() override { return true; }

    /**
     * @brief Does nothing.
     *
     * This is a default (empty) implementation of init() for plugins.
     */
    void deinit() override { }

private:
    /**
     * @brief Extracts class meta information with given key.
     * @param key Key to extract.
     * @return Value of the meta information, or null if there's no information with given key.
     *
     * This is a helper method which queries Qt's meta object subsystem for class meta information defined with Q_CLASSINFO.
     */
    const char *metaInfo(const QString &key) const;
};

inline QString QBuiltInPlugin::title() const
{
    const char *title = metaInfo("title");
    return nullptr == title ? name() : title;
}

#define Q_BUILD_IN_PLUGIN(Title, Desc, FormName, Version, Author) \
    Q_CLASSINFO("title", Title) \
    Q_CLASSINFO("description", Desc) \
    Q_CLASSINFO("ui", FormName) \
    Q_CLASSINFO("version", #Version) \
    Q_CLASSINFO("author", Author)

#endif // QBUILTINPLUGIN_H
