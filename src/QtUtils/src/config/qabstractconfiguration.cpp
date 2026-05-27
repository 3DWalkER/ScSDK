#include "qtutils/config/qabstractconfiguration.h"

#include <QDir>
#include <QSet>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QDebug>

QSettings *CONFIG_SETTINGS_INSTANCE = nullptr;

QAbstractConfiguration::~QAbstractConfiguration()
{

}

QString QAbstractConfiguration::portablePath()
{
    QStringList paths({ qApp->applicationDirPath() + "/config", "./config"});
    QSet<QString> pathsSet;
    QDir dir;
    for (const QString &path : paths)
    {
        dir.setPath(path);
        pathsSet << dir.absolutePath();
    }

    QFileInfo fileInfo;
    QString potentialPath, absolutePath;
    for (const QString &path : pathsSet)
    {
        dir.setPath(path);
        absolutePath = dir.absolutePath();
        fileInfo.setFile(absolutePath);
        if (!fileInfo.exists())
        {
            if (potentialPath.isEmpty())
                potentialPath = absolutePath;

            continue;
        }

        if (!fileInfo.isDir() || !fileInfo.isReadable() || !fileInfo.isWritable())
            continue;

        return absolutePath;
    }

    return potentialPath;
}

QString QAbstractConfiguration::appConfigPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QSettings *QAbstractConfiguration::settings()
{
    if (nullptr == CONFIG_SETTINGS_INSTANCE)
    {
        QString portablePath = QAbstractConfiguration::portablePath();
        QFileInfo fileInfo(portablePath);
        if (fileInfo.exists() && fileInfo.isDir() && fileInfo.isReadable())
            CONFIG_SETTINGS_INSTANCE = new QSettings(portablePath + "/settings.ini", QSettings::IniFormat);
        else
            CONFIG_SETTINGS_INSTANCE = new QSettings();
    }

    return CONFIG_SETTINGS_INSTANCE;
}
