#include "qtutils/config/qconfiglazyinitializer.h"

#include <QList>

class QConfigLazyInitializerPrivate
{
    Q_DECLARE_PUBLIC(QConfigLazyInitializer)
public:
    QConfigLazyInitializerPrivate(QConfigLazyInitializer *q, std::function<void(void)> initFunc)
        : q_ptr(q)
        , m_pInitFunc(initFunc)
    {
        if (nullptr == m_pInstance)
            m_pInstance = new QList<QConfigLazyInitializer *>();

        *m_pInstance << q;
    }

    inline void doInitialize() { if (nullptr != m_pInitFunc) m_pInitFunc(); }

    QConfigLazyInitializer *q_ptr;
    std::function<void(void)> m_pInitFunc;

    static QList<QConfigLazyInitializer *> *m_pInstance;
};

QList<QConfigLazyInitializer *> *QConfigLazyInitializerPrivate::m_pInstance = nullptr;

QConfigLazyInitializer::QConfigLazyInitializer(std::function<void(void)> initFunc)
    : d_ptr(new QConfigLazyInitializerPrivate(this, initFunc))
{

}

QConfigLazyInitializer::~QConfigLazyInitializer()
{
    delete d_ptr;
}

void QConfigLazyInitializer::init()
{
    if (nullptr == QConfigLazyInitializerPrivate::m_pInstance)
        return;

    for (QConfigLazyInitializer *&initializer : *QConfigLazyInitializerPrivate::m_pInstance)
        initializer->d_func()->doInitialize();
}
