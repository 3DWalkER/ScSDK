#include "frameless/elements/qstandardtitlebar.h"

#include "frameless/elements/qsystembutton.h"
#include "frameless/utils/qframelessstyle_p.h"
#include "frameless/utils/qframelesshelper.h"

#include <QBoxLayout>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QPointer>
#include <QTimer>
#include <QtMath>
#include <QDebug>

class QStandardTitleBarPrivate
{
    Q_DECLARE_PUBLIC(QStandardTitleBar)
public:
    explicit QStandardTitleBarPrivate(QStandardTitleBar *q);

    void initialize();
    void onWindowChanged();

    void setFixedHeight();

    QList<QSystemButton *> buttons() const;

    QPoint labelPosition() const;
    int labelMaxWidth() const;

    bool isWindowIconRealVisible() const;
    QRect windowIconRect() const;

    QStandardTitleBar *q_ptr;
    QPointer<QWidget> window;
    QPointer<QFramelessStyle> m_pStyle { nullptr };

    QSystemButton *m_pMiniBtn { };
    QSystemButton *m_pMaxiBtn { };
    QSystemButton *m_pCloseBtn { };

    bool isExtensible { true };
    bool isExtended { true };
    int normalHeight { QF::kDefaultTitleHeight };
    int extendedHeight { QF::kDefaultTitleBarExtendedHeight };

    Qt::Alignment labelAlignment { Qt::AlignLeft };
    bool isLabelVisible { true };
    bool isIconVisible { true };

    bool isHideWhenClose { };
    QVector<QMetaObject::Connection> connects;
};

QStandardTitleBar::QStandardTitleBar(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new QStandardTitleBarPrivate(this))
{
    Q_D(QStandardTitleBar);
    d->initialize();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAttribute(Qt::WA_TranslucentBackground);
}

QStandardTitleBar::~QStandardTitleBar()
{
    delete d_ptr;
}

QSystemButton *QStandardTitleBar::minimizeButton() const
{
    Q_D(const QStandardTitleBar);
    return d->m_pMiniBtn;
}

QSystemButton *QStandardTitleBar::maximizeButton() const
{
    Q_D(const QStandardTitleBar);
    return d->m_pMaxiBtn;
}

QSystemButton *QStandardTitleBar::closeButton() const
{
    Q_D(const QStandardTitleBar);
    return d->m_pCloseBtn;
}

bool QStandardTitleBar::isExtensible() const
{
    Q_D(const QStandardTitleBar);
    return d->isExtensible;
}

void QStandardTitleBar::setExtensible(bool on)
{
    Q_D(QStandardTitleBar);
    if (on == d->isExtensible)
        return;

    d->isExtensible = on;
    if (!on)
    {
        d->isExtended = false;
        d->setFixedHeight();
    }
    emit extensibleChanged(on);
}

bool QStandardTitleBar::isExtended() const
{
    Q_D(const QStandardTitleBar);
    if (!d->isExtensible)
        return false;
    return d->isExtended;
}

void QStandardTitleBar::setExtended(bool on)
{
    Q_D(QStandardTitleBar);
    if (on == d->isExtended || !d->isExtensible)
        return;

    d->isExtended = on;
    d->setFixedHeight();
}

int QStandardTitleBar::extendedHeight() const
{
    Q_D(const QStandardTitleBar);
    return d->extendedHeight;
}

void QStandardTitleBar::setExtendedHeight(int height)
{
    Q_D(QStandardTitleBar);
    if (height == d->extendedHeight)
        return;

    d->extendedHeight = height;
    if (d->isExtended)
        setFixedHeight(height);
}

Qt::Alignment QStandardTitleBar::labelAlignment() const
{
    Q_D(const QStandardTitleBar);
    return d->labelAlignment;
}

void QStandardTitleBar::setLabelAlignment(Qt::Alignment align)
{
    Q_D(QStandardTitleBar);
    if (align == d->labelAlignment)
        return;

    d->labelAlignment = align;
    update();
}

bool QStandardTitleBar::isLabelVisible() const
{
    Q_D(const QStandardTitleBar);
    return d->isLabelVisible;
}

void QStandardTitleBar::setLabelVisible(bool on)
{
    Q_D(QStandardTitleBar);
    if (on == d->isLabelVisible)
        return;

    d->isLabelVisible = on;
    update();
}

bool QStandardTitleBar::isWindowIconVisible() const
{
    Q_D(const QStandardTitleBar);
    return d->isIconVisible;
}

void QStandardTitleBar::setWindowIconVisible(bool on)
{
    Q_D(QStandardTitleBar);
    if (on == d->isIconVisible)
        return;

    d->isIconVisible = on;
    update();
}

void QStandardTitleBar::setUsingQss(bool on)
{
    Q_D(QStandardTitleBar);
    for (QSystemButton *pBut : d->buttons())
        pBut->setUsingQss(on);
    update();
}

bool QStandardTitleBar::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::ParentChange:
    {
        Q_D(QStandardTitleBar);
        if (d->window != window())
        {
            if (d->m_pStyle && this == d->window)
                d->m_pStyle->release();
            d->onWindowChanged();
        }
        break;
    }
    case QEvent::Show:
    {
        Q_D(QStandardTitleBar);
        d->m_pMaxiBtn->setEnabled(!QFramelessHelper::isWidgetFixedSize(window()));
        break;
    }
    default:
        break;
    }
    return QWidget::event(event);
}

void QStandardTitleBar::paintEvent(QPaintEvent *event)
{
    Q_D(QStandardTitleBar);
    if (!d->window)
        return;

    const auto pStyle = d->m_pStyle;
    const bool isActive = d->window->isActiveWindow();
    const bool isUsingQss = pStyle ? pStyle->isTitleBarUsingQss() : true;

    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (!isUsingQss)
    {
        const QColor backgroundColor = isActive ? pStyle->titleBarActiveBackgroundColor() : pStyle->titleBarInactiveBackgroundColor();
        painter.fillRect(QRect(QPoint(0, 0), size()), backgroundColor);
    }

    if (d->isLabelVisible)
    {
        const QString text = d->window->windowTitle();
        if (!isUsingQss)
        {
            const QColor foregroundColor = isActive ? pStyle->titleBarActiveForegroundColor() : pStyle->titleBarInactiveForegroundColor();
            painter.setPen(foregroundColor);
        }
        if (pStyle)
            painter.setFont(pStyle->titleBarFont());

        const int textMaxWidth = d->labelMaxWidth();
        const QString elidedText = painter.fontMetrics().elidedText(text, Qt::ElideRight, textMaxWidth, Qt::TextShowMnemonic);
        if (elidedText.size() > 3)
            painter.drawText(d->labelPosition(), elidedText);
    }

    if (d->isIconVisible)
    {
        const QIcon icon = d->window->windowIcon();
        if (!icon.isNull())
            icon.paint(&painter, d->windowIconRect());
    }

    painter.restore();
    event->accept();
}

QStandardTitleBarPrivate::QStandardTitleBarPrivate(QStandardTitleBar *q)
    : q_ptr(q)
    , m_pMiniBtn(new QSystemButton(QSystemButton::Minimize))
    , m_pMaxiBtn(new QSystemButton(QSystemButton::Maximize))
    , m_pCloseBtn(new QSystemButton(QSystemButton::Close))
{

}

static inline void emulateLeaveEvent(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget)
        return;

    QTimer::singleShot(0, widget, [widget]() {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        const QScreen *screen = widget->screen();
#else
        const QScreen *screen = widget->windowHandle()->screen();
#endif
        const QPoint globalPos = QCursor::pos(screen);
        if (!QRect(widget->mapToGlobal(QPoint{ 0, 0 }), widget->size()).contains(globalPos)) {
            QCoreApplication::postEvent(widget, new QEvent(QEvent::Leave));
            if (widget->testAttribute(Qt::WA_Hover))
            {
                const QPoint localPos = widget->mapFromGlobal(globalPos);
                const QPoint scenePos = widget->window()->mapFromGlobal(globalPos);
                static constexpr const auto oldPos = QPoint{};
                const Qt::KeyboardModifiers modifiers = QGuiApplication::keyboardModifiers();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
                const auto event =  new QHoverEvent(QEvent::HoverLeave, scenePos, globalPos, oldPos, modifiers);
                Q_UNUSED(localPos);
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, globalPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#else
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#endif
                QCoreApplication::postEvent(widget, event);
            }
        }
    });
}

void QStandardTitleBarPrivate::initialize()
{
    Q_Q(QStandardTitleBar);
    QHBoxLayout *pBtnsLayout = new QHBoxLayout();
    pBtnsLayout->setSpacing(0);
    pBtnsLayout->setMargin(0);
    pBtnsLayout->addWidget(m_pMiniBtn);
    pBtnsLayout->addWidget(m_pMaxiBtn);
    pBtnsLayout->addWidget(m_pCloseBtn);

    QHBoxLayout *pTitleLayout = new QHBoxLayout(q);
    pTitleLayout->setSpacing(0);
    pTitleLayout->setContentsMargins(0, 0, 0, 0);
    pTitleLayout->addStretch();
    pTitleLayout->addLayout(pBtnsLayout);
    setFixedHeight();

    onWindowChanged();
}

void QStandardTitleBarPrivate::onWindowChanged()
{
    Q_Q(QStandardTitleBar);
    for (QMetaObject::Connection &connect : connects)
        q->disconnect(connect);

    window = q->window();
    if (!window)
        return;

    m_pStyle = QFramelessStyle::instanceForWindow(window);
    connects << q->connect(m_pStyle, &QFramelessStyle::titleBarUsingQssChanged, q, &QStandardTitleBar::setUsingQss);

    connects << q->connect(m_pMiniBtn, &QSystemButton::clicked, window, &QWidget::showMinimized);
    connects << q->connect(m_pMaxiBtn, &QSystemButton::clicked, window, [this]() {
        window->isMaximized() ? window->showNormal() : window->showMaximized();
        emulateLeaveEvent(m_pMaxiBtn);
    });
    connects << q->connect(m_pCloseBtn, &QSystemButton::clicked, window, [this]() {
        if (isHideWhenClose) window->hide();
        else  window->close();
    });

    connects << q->connect(window, &QWidget::windowIconChanged, q, [q](const QIcon &) { q->update(); });
    connects << q->connect(window, &QWidget::windowTitleChanged, q, [q](const QString &) {  q->update(); });

    QFramelessHelper *pHelper = QFramelessHelper::instance(window);
    if (pHelper)
        m_pMaxiBtn->setEnabled(!pHelper->isWindowFixedSize());
}

inline void QStandardTitleBarPrivate::setFixedHeight()
{
    Q_Q(QStandardTitleBar);
    int fixedHeight = isExtended ? extendedHeight : normalHeight;
    q->setFixedHeight(fixedHeight);
    for (QSystemButton *button : buttons())
        button->setFixedHeight(fixedHeight);
}

QList<QSystemButton *> QStandardTitleBarPrivate::buttons() const
{
    Q_Q(const QStandardTitleBar);
    return q->findChildren<QSystemButton *>("", Qt::FindDirectChildrenOnly);
}

QPoint QStandardTitleBarPrivate::labelPosition() const
{
    Q_Q(const QStandardTitleBar);
    QString text = window->windowTitle();
    if (text.isEmpty())
        return { };

    const QFontMetrics metrics(m_pStyle->titleBarFont());
    const int fontWidth = metrics.horizontalAdvance(text);
    const int titleBarWidth = q->width();

    int x = 0;
    int contentsMargin = m_pStyle->titleBarContentsMargin();
    if (labelAlignment & Qt::AlignLeft)
        x = windowIconRect().right() + contentsMargin;

    const int y = std::round((qreal(q->height() - metrics.height()) / qreal(2)) + qreal(metrics.ascent()));
    return { x, y };
}

int QStandardTitleBarPrivate::labelMaxWidth() const
{
    Q_Q(const QStandardTitleBar);
    int textMaxWidth = q->width();
    const int contentsMargin = m_pStyle->titleBarContentsMargin();
    const int btnAreaWidth = buttons().size() * m_pStyle->buttonWidth();
    if ((labelAlignment & Qt::AlignLeft) || (labelAlignment & Qt::AlignRight))
        textMaxWidth -= windowIconRect().width() + btnAreaWidth + (contentsMargin * 2);
    else if (labelAlignment & Qt::AlignHCenter)
        textMaxWidth -= (btnAreaWidth + contentsMargin) * 2;
    else
        textMaxWidth = std::round(qreal(textMaxWidth) * qreal(0.8));
    return std::max(textMaxWidth, 0);
}

bool QStandardTitleBarPrivate::isWindowIconRealVisible() const
{
    return isIconVisible && !window->windowIcon().isNull();
}

QRect QStandardTitleBarPrivate::windowIconRect() const
{
    if (!isWindowIconRealVisible())
        return { };

    Q_Q(const QStandardTitleBar);
    const QSize size = m_pStyle->windowIconSize();

#ifdef Q_OS_MACOS
#else
    const int x = m_pStyle->titleBarContentsMargin();
#endif
    const int y = std::round(qreal(q->height() - size.height()) / qreal(2));
    return { QPoint(x, y), size };
}
