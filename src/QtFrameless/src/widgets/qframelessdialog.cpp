#include "frameless/widgets/qframelessdialog.h"

#include "frameless/widgets/qwidgetscontainer.h"

#include <QDebug>

class QFramelessDialogPrivate
{
public:
    explicit QFramelessDialogPrivate(QFramelessDialog *q, QWidget *parent);
    ~QFramelessDialogPrivate();

    QFramelessDialog *q_ptr;
    QWidgetsContainer *m_pSharedHelper { };
};

QFramelessDialog::QFramelessDialog(QWidget *parent)
    : QDialog(nullptr)
    , d_ptr(new QFramelessDialogPrivate(this, parent))
{
    Q_D(QFramelessDialog);
    d->m_pSharedHelper->setup(this);
}

QFramelessDialog::~QFramelessDialog()
{
    delete d_ptr;
}

int QFramelessDialog::exec()
{
    Q_D(QFramelessDialog);
    d->m_pSharedHelper->exec();
    return result();
}

QFramelessDialogPrivate::QFramelessDialogPrivate(QFramelessDialog *q, QWidget *parent)
    : q_ptr(q)
    , m_pSharedHelper(new QWidgetsContainer(parent))
{

}

QFramelessDialogPrivate::~QFramelessDialogPrivate()
{
    q_ptr->setParent(nullptr);
    if (nullptr != m_pSharedHelper)
    {
        delete m_pSharedHelper;
        m_pSharedHelper = nullptr;
    }
}
