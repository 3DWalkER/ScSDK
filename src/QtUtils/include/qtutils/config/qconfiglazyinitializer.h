#ifndef QCONFIGLAZYINITIALIZER_H
#define QCONFIGLAZYINITIALIZER_H

#include <functional>
#include "qtutils/qutilsglobal.h"

class QConfigLazyInitializerPrivate;

class QU_API_EXPORT QConfigLazyInitializer
{
    Q_DECLARE_PRIVATE(QConfigLazyInitializer)
public:
    explicit QConfigLazyInitializer(std::function<void(void)> initFunc);
    virtual ~QConfigLazyInitializer();

    static void init();

private:
    QConfigLazyInitializerPrivate *d_ptr;
};

#endif // QCONFIGLAZYINITIALIZER_H
