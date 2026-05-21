#include "frameless/widgets/qwidgetscontainer.h"

#include "frameless/elements/qsystembutton.h"
#include "frameless/elements/qstandardtitlebar.h"
#include "frameless/elements/qwindowborder.h"
#include "frameless/utils/qframelessstyle.h"
#include "frameless/utils/qframelesshelper.h"
#include "frameless/utils/qsystemversion_p.h"
#include "frameless/utils/qframelessutils_p.h"

#include <QBoxLayout>
#include <QKeyEvent>
#include <QMainWindow>
#include <QEventLoop>
#include <QDialog>
#include <QPainter>
#include <QPointer>
#include <QDebug>

class QWidgetsContainerPrivate
{
    Q_DECLARE_PUBLIC(QWidgetsContainer)
public:
    explicit QWidgetsContainerPrivate(QWidgetsContainer* q);
    ~QWidgetsContainerPrivate();

    QWidgetsContainer* q_ptr;

    QPointer<QWidget> m_pTargetWidget;
    Qt::WindowType windowType{ Qt::Widget };
    QPointer<QWidget> m_pTitleBar;
    QPointer<QFramelessStyle> m_pStype;
    QWindowBorder* m_pWindowBorder{ };
    QEventLoop* eventLoop{ };

    void _q_setWindowBorderEnabled(bool on);

    void checkWidgetType();
    bool isWindowBorderEnabled() const;

    bool containerEventFilter(QEvent* event);
    bool targetWidgetEventFilter(QEvent* event);
};

QWidgetsContainer::QWidgetsContainer(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new QWidgetsContainerPrivate(this))
{

}

QWidgetsContainer::~QWidgetsContainer()
{
    delete d_ptr;
}

void QWidgetsContainer::setup(QWidget* widget)
{
    Q_ASSERT(widget);
    if (!widget) return;

    Q_D(QWidgetsContainer);
    if (d->m_pTargetWidget)
    {
        qWarning() << __FUNCTION__ << "Setup failed: The target widget already has a layout structure!";
        return;
    }

    if (widget == d->m_pTargetWidget)
        return;

    d->m_pTargetWidget = widget;
    d->m_pStype = QFramelessStyle::instance(widget);
    d->m_pTargetWidget->installEventFilter(this);
    d->checkWidgetType();

    QFramelessHelper* pFramelessHelper = QFramelessHelper::instance(this);
    pFramelessHelper->extends(true);

    initLayout(widget);

    resize(400, 300);
    installEventFilter(this);

    d->_q_setWindowBorderEnabled(d->m_pStype->isWindowBorderEnabled());
    connect(d->m_pStype, SIGNAL(windowBorderEnabledChanged(bool)), this, SLOT(_q_setWindowBorderEnabled(bool)));
    connect(d->m_pTargetWidget, &QWidget::windowIconChanged, this, [this](const QIcon &icon) { setWindowIcon(icon); });
}

void QWidgetsContainer::exec()
{
    bool wasShowModal = testAttribute(Qt::WA_ShowModal);
    setAttribute(Qt::WA_ShowModal, true);

    show();

    Q_D(QWidgetsContainer);
    QEventLoop eventLoop;
    d->eventLoop = &eventLoop;
    eventLoop.exec(QEventLoop::DialogExec);
    setAttribute(Qt::WA_ShowModal, wasShowModal);
}

QWidget *QWidgetsContainer::widget() const
{
    Q_D(const QWidgetsContainer);
    return d->m_pTargetWidget;
}

QWidget *QWidgetsContainer::titleBarWidget() const
{
    Q_D(const QWidgetsContainer);
    return d->m_pTitleBar;
}

void QWidgetsContainer::setTitleBarWidget(QWidget *widget)
{
    Q_D(QWidgetsContainer);
    if (widget == d->m_pTitleBar)
        return;

    if (d->m_pTitleBar)
    {
        delete d->m_pTitleBar;
        d->m_pTitleBar = nullptr;
    }

    if (!widget)
        return;

    d->m_pTitleBar = widget;
    const int height = windowState() & Qt::WindowNoState
            ? d->m_pStype->titleBarNormalHeight()
            : d->m_pStype->titleBarExtendedHeight();
    d->m_pTitleBar->setFixedHeight(height);
    QVBoxLayout* pMainLayout  = qobject_cast<QVBoxLayout *>(layout());
    pMainLayout->insertWidget(0, d->m_pTitleBar);
}

void QWidgetsContainer::initLayout(QWidget* widget)
{
    Q_D(QWidgetsContainer);
    QVBoxLayout* pMainLayout = new QVBoxLayout(this);
    pMainLayout->setContentsMargins(d->m_pStype->contentsMargins());
    pMainLayout->setSpacing(0);

    d->m_pTitleBar = new QStandardTitleBar(this);
    pMainLayout->addWidget(d->m_pTitleBar);
    pMainLayout->addWidget(widget);

    QFramelessHelper* pHelper = QFramelessHelper::instance(this);
    pHelper->setTitleBarWidget(d->m_pTitleBar);
    QStandardTitleBar *pTitleBar = qobject_cast<QStandardTitleBar *>(d->m_pTitleBar);
    if (pTitleBar)
    {
        pHelper->setSystemButton(pTitleBar->minimizeButton(), QSystemButton::Minimize);
        pHelper->setSystemButton(pTitleBar->maximizeButton(), QSystemButton::Maximize);
        pHelper->setSystemButton(pTitleBar->closeButton(), QSystemButton::Close);
    }
}

bool QWidgetsContainer::eventFilter(QObject* object, QEvent* event)
{
    Q_D(QWidgetsContainer);
    if (object == d->m_pTargetWidget)
        return d->targetWidgetEventFilter(event);

    if (this == object)
        return d->containerEventFilter(event);
    return QWidget::eventFilter(object, event);
}

QWidgetsContainerPrivate::QWidgetsContainerPrivate(QWidgetsContainer* q)
    : q_ptr(q)
{

}

QWidgetsContainerPrivate::~QWidgetsContainerPrivate()
{
    if (m_pStype)
        m_pStype->release();
}

void QWidgetsContainerPrivate::_q_setWindowBorderEnabled(bool on)
{
    Q_Q(QWidgetsContainer);
    if (on)
    {
        if (!m_pWindowBorder)
        {
            m_pWindowBorder = new QWindowBorder(q);
            m_pWindowBorder->setThickness(m_pStype->windowBorderThickness());
            m_pWindowBorder->setActiveColor(m_pStype->windowBorderActiveColor());
            m_pWindowBorder->setInactiveColor(m_pStype->windowBorderInactiveColor());

            QLayout* pLayout = q->layout();
            if (pLayout)
                pLayout->setContentsMargins(m_pStype->contentsMargins());

            q->connect(m_pWindowBorder, &QWindowBorder::shouldRepaint, q, [q]() { q->update(); });
            q->connect(m_pWindowBorder, &QWindowBorder::thicknessChanged, q, [q, this]() {
                q->layout()->setContentsMargins(m_pStype->contentsMargins());
            });
            q->connect(m_pStype, &QFramelessStyle::windowBorderThicknessChanged, m_pWindowBorder, &QWindowBorder::setThickness);
            q->connect(m_pStype, &QFramelessStyle::windowBorderActiveColorChanged, m_pWindowBorder, &QWindowBorder::setActiveColor);
            q->connect(m_pStype, &QFramelessStyle::windowBorderInactiveColorChanged, m_pWindowBorder, &QWindowBorder::setInactiveColor);
        }
    }
    else
    {
        if (nullptr != m_pWindowBorder)
        {
            q->layout()->setContentsMargins(m_pStype->contentsMargins());
            delete m_pWindowBorder;
            m_pWindowBorder = nullptr;
        }
    }
}

void QWidgetsContainerPrivate::checkWidgetType()
{
    Q_Q(QWidgetsContainer);
    if (nullptr != qobject_cast<QDialog*>(m_pTargetWidget))
    {
        windowType = Qt::Dialog;
        q->setWindowFlags(Qt::Dialog);
    }
    else if (nullptr != qobject_cast<QMainWindow*>(m_pTargetWidget))
        windowType = Qt::Window;
    else
        windowType = Qt::Widget;
}

bool QWidgetsContainerPrivate::isWindowBorderEnabled() const
{
    return nullptr != m_pWindowBorder && Qt::WindowNoState == q_func()->windowState()
            && m_pStype->isWindowBorderEnabled();
}

bool QWidgetsContainerPrivate::containerEventFilter(QEvent* event)
{
    Q_Q(QWidgetsContainer);
    switch (event->type())
    {
    case QEvent::KeyPress:
    {
        if (Qt::Dialog == windowType)
        {
            QKeyEvent* pKeyEvent = dynamic_cast<QKeyEvent*>(event);
            if (pKeyEvent->matches(QKeySequence::Cancel))
            {
                QDialog* pDialog = static_cast<QDialog*>(m_pTargetWidget.data());
                pDialog->reject();
                if (nullptr != eventLoop)
                    eventLoop->exit();
                eventLoop = nullptr;
            }
        }
        break;
    }
    case QEvent::Close:
    {
        if (nullptr != eventLoop)
        {
            eventLoop->exit();
            eventLoop = nullptr;
        }
        break;
    }
    case QEvent::WindowStateChange:
    {
        Qt::WindowStates state = q->windowState();
        QStandardTitleBar *pTitleBar = qobject_cast<QStandardTitleBar *>(m_pTitleBar);
        if (nullptr != m_pTitleBar)
        {
            QSystemButton* pMaxBtn = pTitleBar->maximizeButton();
            if (nullptr != pMaxBtn)
                pMaxBtn->setButtonType(Qt::WindowMaximized == state ? QSystemButton::Restore : QSystemButton::Maximize);
        }

        if (Qt::WindowMaximized == state)
        {
            if (pTitleBar)
                pTitleBar->setExtended(false);
            q->layout()->setContentsMargins(QMargins());
        }
        else if (Qt::WindowNoState == state)
        {
            if (pTitleBar)
                pTitleBar->setExtended(true);
            q->layout()->setContentsMargins(m_pStype->contentsMargins());
        }
        break;
    }
    case QEvent::ActivationChange:
        if (isWindowBorderEnabled())
            m_pStype->updateWindowBorderByActiveState();
        break;
    case QEvent::Paint:
    {
        if (isWindowBorderEnabled())
        {
            if (m_pWindowBorder->thickness() > 1 || !QSystemVersion::isWin11OrGreater())
            {
                const WId wid = q->winId();
                int radius = qMax(0, QF::windowCornerRadius(wid));
                if (radius > 0)
                {
                    if (QF::isWindowAttachedToEdge(wid))
                        radius = 0;
                }
                QPainter painter(q);
                m_pWindowBorder->paint(&painter, q->size(), q->isActiveWindow(), radius);
            }
        }
        break;
    }
    default:
        break;
    }

    return false;
}

bool QWidgetsContainerPrivate::targetWidgetEventFilter(QEvent* event)
{
    Q_Q(QWidgetsContainer);
    switch (event->type())
    {
    case QEvent::ShowToParent:
    {
        Qt::WindowStates states = m_pTargetWidget->windowState();
        switch (states)
        {
        case Qt::WindowMinimized:
            q->showMinimized();
            break;
        case Qt::WindowMaximized:
            q->showMaximized();
            break;
        case Qt::WindowFullScreen:
            q->showFullScreen();
            break;
        default:
            q->show();
            break;
        }
        return true;
    }
    case QEvent::WindowStateChange:
        q->setWindowState(m_pTargetWidget->windowState());
        return true;
    case QEvent::HideToParent:
        q->close();
        return true;
    case QEvent::WindowTitleChange:
        q->setWindowTitle(m_pTargetWidget->windowTitle());
        return true;
    case QEvent::StyleChange:
        q->setStyleSheet(m_pTargetWidget->styleSheet());
        return true;
    default:
        break;
    }
    return false;
}

#include "frameless/widgets/moc_qwidgetscontainer.cpp"
