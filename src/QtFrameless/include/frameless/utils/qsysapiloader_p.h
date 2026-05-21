#ifndef QSYSAPILOADER_H
#define QSYSAPILOADER_H

#include <QtGlobal>

namespace QSysApiLoader
{

QString sharedLibrarySuffix();
QString sharedLibraryDirectory();
QString generateUniqueKey(const QString &library, const QString &function);

QFunctionPointer resolve(const QString &library, const char *function);
QFunctionPointer resolve(const QString &library, const QString &function);

bool isAvailable(const QString &library, const QString &function);
QFunctionPointer get(const QString &library, const QString &function);

template<typename T>
T get(const QString &library, const QString &function) {
    return reinterpret_cast<T>(get(library, function));
}

}

#endif // QSYSAPILOADER_H
