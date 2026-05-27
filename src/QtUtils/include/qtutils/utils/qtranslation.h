#ifndef QTRANSLATION_H
#define QTRANSLATION_H

#include "qtutils/qutilsglobal.h"
#include <QMap>

QU_BEGIN_NAMESPACE

/**
 * @brief setTranslationDirs 设置翻译文件所在目录
 * @note 默认为{ "translations", "../../resources/translations", "./resources/translations", ":/translations", ":/resources/translations" }
 * 此函数在该命名空间下其它函数调用前调用才起作用。
 */
QU_API_EXPORT void setTranslationDirs(const QStringList& dirs);

/**
 * @brief loadTranslation 加载翻译文件
 * @param baseName 翻译文件的基础名称，如“qt_en.qm”中的“qt”
 * @param lang 语言类型，如“qt_en.qm”中的“en”
 */
QU_API_EXPORT void loadTranslation(const QString& baseName, const QString& lang);

/**
 * @brief loadTranslations 加载多个翻译文件
 */
QU_API_EXPORT void loadTranslations(const QStringList& baseNames, const QString& lang);

/**
 * @brief unloadTranslation 卸载指定的翻译文件
 */
QU_API_EXPORT void unloadTranslation(const QString& baseName);

/**
 * @brief systemLanguage 获取系统当前语言
 */
QU_API_EXPORT QString systemLanguage();

/**
 * @brief availableTranslations 获取支持的翻译文件
 */
QU_API_EXPORT QStringList availableTranslations();

/**
 * @brief availableTranslations 获取支持的语言
 * @return { 语言名称, 语言代码 }
 */
QU_API_EXPORT QMap<QString, QString> availableLanguages();

QU_END_NAMESPACE

#endif // QTRANSLATION_H
