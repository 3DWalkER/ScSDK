#ifndef QAPPSTUDIO_H
#define QAPPSTUDIO_H

#include "qtutils/qutilsglobal.h"

class QAppStudioPrivate;

class QU_API_EXPORT QAppStudio
{
    Q_DECLARE_SINGLETON(QAppStudio)
public:
    ~QAppStudio();

private:
    QAppStudio();

    Q_DECLARE_PRIVATE(QAppStudio)
    QAppStudioPrivate *d_ptr;
};

#define QU_APPSTUDIO QAppStudio::instance()

#endif // QAPPSTUDIO_H
