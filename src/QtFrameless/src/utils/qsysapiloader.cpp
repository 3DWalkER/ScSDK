#include "frameless/utils/qsysapiloader_p.h"

#include <QVarLengthArray>
#include <QDir>
#include <QtCore/private/qsystemlibrary_p.h>

using QSysApiLoaderData = QHash<QString, QFunctionPointer>;
Q_GLOBAL_STATIC(QSysApiLoaderData, g_sysApiLoaderData)

QString QSysApiLoader::sharedLibrarySuffix()
{
    static const auto result = []() -> QString {
        #ifdef Q_OS_WINDOWS
            return QStringLiteral(".dll");
        #elif (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
            return QStringLiteral(".so");
        #elif defined(Q_OS_MACOS)
            return QStringLiteral(".dylib");
        #else
        #  error "Unsupported platform!"
        #endif
}();
    return result;
}

QString QSysApiLoader::sharedLibraryDirectory()
{
    static const auto result = []() -> QString {
        #ifdef Q_OS_WINDOWS
            QVarLengthArray<wchar_t, MAX_PATH> buf = {};
            const UINT len = ::GetSystemDirectoryW(buf.data(), MAX_PATH);
            if (len > MAX_PATH)
            {
                buf.resize(len);
                ::GetSystemDirectoryW(buf.data(), len);
            }
            return QString::fromWCharArray(buf.constData(), len);
        #else
            return "/usr/lib";
        #endif
}();
    return result;
}

QString QSysApiLoader::generateUniqueKey(const QString &library, const QString &function)
{
    if (library.isEmpty() || function.isEmpty())
        return { };

    QString key = QDir::toNativeSeparators(library);
    const qsizetype lastSeparatorPos = key.lastIndexOf(QDir::separator());
    if (lastSeparatorPos > 0)
        key.remove(0, lastSeparatorPos);

#ifdef Q_OS_WINDOWS
    key = key.toLower();
#endif

    static const QString suffix = sharedLibrarySuffix();
    if (!key.endsWith(suffix))
        key.append(suffix);
    key.append(u'@');
    key.append(function);
    return key;
}

QFunctionPointer QSysApiLoader::resolve(const QString &library, const char *function)
{
    if (library.isEmpty() || !function)
        return nullptr;
    return QSystemLibrary::resolve(library, function);
}

QFunctionPointer QSysApiLoader::resolve(const QString &library, const QString &function)
{
    if (library.isEmpty() || function.isEmpty())
        return nullptr;
    return QSystemLibrary::resolve(library, function.toUtf8().constData());
}

bool QSysApiLoader::isAvailable(const QString &library, const QString &function)
{
    if (library.isEmpty() || function.isEmpty())
        return false;

    const QString key = generateUniqueKey(library, function);
    const auto it = g_sysApiLoaderData()->constFind(key);
    if (g_sysApiLoaderData->constEnd() == it)
    {
        const QFunctionPointer symbol = QSysApiLoader::resolve(library, function);
        g_sysApiLoaderData()->insert(key, symbol);
        return nullptr != symbol;
    }
    else
        return nullptr != it.value();
}

QFunctionPointer QSysApiLoader::get(const QString &library, const QString &function)
{
    if (library.isEmpty() || function.isEmpty())
        return nullptr;

    const QString key = generateUniqueKey(library, function);
    const auto it = g_sysApiLoaderData()->constFind(key);
    if (g_sysApiLoaderData->constEnd() == it)
    {
        const QFunctionPointer symbol = QSysApiLoader::resolve(library, function);
        g_sysApiLoaderData()->insert(key, symbol);
        return symbol;
    }
    else
        return it.value();
}
