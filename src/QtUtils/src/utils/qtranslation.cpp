#include "qtranslation.h"

#include <QDir>
#include <QSet>
#include <QHash>
#include <QLocale>
#include <QTranslator>
#include <QCoreApplication>
#include <QRegularExpression>

QHash <QString, QTranslator *> QU_TRANSLATIONS;
QStringList QU_TRANSLATION_DIRS{ "translations", "../../resources/translations", "./resources/translations", ":/translations", ":/resources/translations" };

void QTranslation::setTranslationDirs(const QStringList &dirs)
{
    QU_TRANSLATION_DIRS = dirs;
}

void QTranslation::loadTranslation(const QString &baseName, const QString &lang)
{
    if (QU_TRANSLATIONS.contains(baseName))
        return;

    QTranslator *translator = new QTranslator();

    QDir dir;
    bool isSucceed = false;
    QStringList filters({ baseName + "_" + lang + ".qm" });
    for (const QString &path : QU_TRANSLATION_DIRS)
    {
        dir.setPath(path);
        for (const QString &entry : dir.entryList(filters))
        {
            isSucceed = translator->load(entry, path);
            if (isSucceed)
            {
                qApp->installTranslator(translator);
                QU_TRANSLATIONS[baseName] = translator;
                return;
            }
        }
    }

    if (!isSucceed)
        delete translator;
}

void QTranslation::loadTranslations(const QStringList &baseNames, const QString &lang)
{
    for (const QString &baseName : baseNames)
        loadTranslation(baseName, lang);
}

void QTranslation::unloadTranslation(const QString &baseName)
{
    if (!QU_TRANSLATIONS.contains(baseName))
        return;

    QTranslator *translator = QU_TRANSLATIONS[baseName];
    if (nullptr != translator)
    {
        qApp->removeTranslator(translator);
        delete translator;
        translator = nullptr;
    }
    QU_TRANSLATIONS.remove(baseName);
}

QString QTranslation::systemLanguage()
{
    return QString();
}

QStringList QTranslation::availableTranslations()
{
    QRegularExpression re("[^\\_]+\\_(\\w+)\\.qm");
    QRegularExpressionMatch match;
    QDir dir;
    QSet<QString> locales;
    for (const QString &path : QU_TRANSLATION_DIRS)
    {
        dir.setPath(path);
        for (const QString entry : dir.entryList({ "*_*.qm" }))
        {
            match = re.match(entry);
            if (!match.isValid())
                continue;

            locales << match.captured(1).toLower();
        }
    }

    locales << "en";
    locales.remove("en_us");

    return locales.values();
}

QMap<QString, QString> QTranslation::availableLanguages()
{
    QMap<QString, QString> langs;

    QLocale locale;
    QString langName;
    QStringList translations = availableTranslations();
    for (const QString &trans : translations)
    {
        locale = QLocale(trans);
        langName = locale.nativeLanguageName();
        if (langName.isEmpty())
            langName = trans;
        langs[trans] = langName;
    }

    return langs;
}
