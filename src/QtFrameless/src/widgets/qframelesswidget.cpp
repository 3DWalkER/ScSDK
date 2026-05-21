#include "frameless/widgets/qframelesswidget.h"

#include "frameless/widgets/qwidgetscontainer.h"

#include <QBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>

class QFramelessWidgetPrivate
{
    Q_DECLARE_PUBLIC(QFramelessWidget)
public:
    explicit QFramelessWidgetPrivate(QFramelessWidget *q);
    ~QFramelessWidgetPrivate();

    QFramelessWidget *q_ptr;
    QWidgetsContainer *m_pSharedHelper { };
};

QFramelessWidget::QFramelessWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new QFramelessWidgetPrivate(this))
{
    Q_D(QFramelessWidget);
    d->m_pSharedHelper->setup(this);
}

QFramelessWidget::~QFramelessWidget()
{
    delete d_ptr;
}

QWidgetsContainer *QFramelessWidget::container() const
{
    Q_D(const QFramelessWidget);
    return d->m_pSharedHelper;
}

QFramelessWidgetPrivate::QFramelessWidgetPrivate(QFramelessWidget *q)
    : q_ptr(q)
    , m_pSharedHelper(new QWidgetsContainer())
{

}

QFramelessWidgetPrivate::~QFramelessWidgetPrivate()
{
    q_func()->setParent(nullptr);
    if (nullptr != m_pSharedHelper)
    {
        delete m_pSharedHelper;
        m_pSharedHelper = nullptr;
    }
}
