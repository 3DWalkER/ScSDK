#include "frameless/utils/qframelessstyle_p.h"

#include "frameless/elements/qstandardtitlebar.h"
#include "frameless/utils/qframelessutils_p.h"
#include "frameless/widgets/qwidgetscontainer.h"
#include "frameless/utils/qsystemversion_p.h"

#include <QGuiApplication>

QFramelessStyle::QFramelessStyle(QWidget *parent)
    : QObject(parent)
    , d_ptr(new QFramelessStylePrivate(this))
{

}

QFramelessStyle::QFramelessStyle(QWidget *parent, QFramelessStyle *target)
    : QObject(parent)
    , d_ptr(new QFramelessStylePrivate(this, parent, target->d_func()))
{

}

QFramelessStyle::~QFramelessStyle()
{
    delete d_ptr;
}

void QFramelessStyle::release()
{
    Q_D(QFramelessStyle);
    QFramelessStylePrivate::instances.remove(d->widget);
    deleteLater();
}

int QFramelessStyle::titleBarNormalHeight() const
{
    Q_D(const QFramelessStyle);
    return d->titleNormalHeight;
}

int QFramelessStyle::titleBarExtendedHeight() const
{
    Q_D(const QFramelessStyle);
    return d->titleExtendedHeight;
}

int QFramelessStyle::titleBarContentsMargin() const
{
    Q_D(const QFramelessStyle);
    return d->titleBarContentsMargin;
}

QFont QFramelessStyle::titleBarFont() const
{
    Q_D(const QFramelessStyle);
    return d->titleBarFont;
}

QSize QFramelessStyle::windowIconSize() const
{
    Q_D(const QFramelessStyle);
    return d->windowIconSize;
}

bool QFramelessStyle::isTitleBarUsingQss() const
{
    Q_D(const QFramelessStyle);
    return d->isTitleBarUsingQss;
}

void QFramelessStyle::setTitleBarUsingQss(bool on)
{
    Q_D(QFramelessStyle);
    if (on == d->isTitleBarUsingQss)
        return;

    d->isTitleBarUsingQss = on;
    if (d->isGlobal)
    {
        for (QFramelessStyle *instance : QFramelessStylePrivate::instances)
            instance->setTitleBarUsingQss(on);
    }
    else
        emit titleBarUsingQssChanged(on);
}

QColor QFramelessStyle::titleBarActiveBackgroundColor() const
{
    return Qt::white;
}

QColor QFramelessStyle::titleBarInactiveBackgroundColor() const
{
    return Qt::white;
}

QColor QFramelessStyle::titleBarActiveForegroundColor() const
{
    return Qt::black;
}

QColor QFramelessStyle::titleBarInactiveForegroundColor() const
{
    return Qt::black;
}

int QFramelessStyle::buttonWidth() const
{
    Q_D(const QFramelessStyle);
    return d->buttonWidth;
}

QColor QFramelessStyle::buttonActiveForegroundColor() const
{
    Q_D(const QFramelessStyle);
    return d->buttonTextActiveColor;
}

QColor QFramelessStyle::buttonInactiveForegroundColor() const
{
    Q_D(const QFramelessStyle);
    return d->buttonTextInactiveColor;
}

QColor QFramelessStyle::closeButtonNormalColor() const
{
    Q_D(const QFramelessStyle);
    return d->closeButtonNormalColor.isValid() ? d->closeButtonNormalColor : QF::kDefaultBtnNormalColor;
}

QColor QFramelessStyle::closeButtonHoverColor() const
{
    Q_D(const QFramelessStyle);
    return d->closeButtonHoverColor.isValid() ? d->closeButtonHoverColor : QF::kDefaultCloseBtnHoverColor;
}

QColor QFramelessStyle::closeButtonPressColor() const
{
    Q_D(const QFramelessStyle);
    return d->closeButtonPressColor.isValid() ? d->closeButtonPressColor : QF::kDefaultCloseBtnHoverColor;
}

void QFramelessStyle::setTitleBarNormalHeight(int height)
{
    Q_D(QFramelessStyle);
    if (height == d->titleNormalHeight)
        return;

    d->titleNormalHeight = height;
    emit titleBarNormalHeightChanged(height);
}

void QFramelessStyle::setTitleBarExtendedHeight(int height)
{
    Q_D(QFramelessStyle);
    if (height == d->titleExtendedHeight)
        return;

    d->titleExtendedHeight = height;
    emit titleBarExtendedHeightChanged(height);
}

bool QFramelessStyle::isWindowBorderEnabled() const
{
    Q_D(const QFramelessStyle);
    return d->isWindowBorderEnabled;
}

int QFramelessStyle::windowBorderThickness() const
{
    Q_D(const QFramelessStyle);
    return d->windowBorderThickness;
}

QColor QFramelessStyle::windowBorderActiveColor() const
{
    Q_D(const QFramelessStyle);
    return d->windowBorderActiveColor;
}

QColor QFramelessStyle::windowBorderInactiveColor() const
{
    Q_D(const QFramelessStyle);
    return d->windowBorderInactiveColor;
}

QMargins QFramelessStyle::contentsMargins() const
{
    int thickness = isWindowBorderEnabled() ? windowBorderThickness() : 0;
    if (0 != thickness)
    {
        thickness /= 2;
        thickness = qMax(1, thickness);
    }

#ifdef Q_OS_WINDOWS
    int top = thickness + 1;
    if (QSystemVersion::isWin11OrGreater())
    {
        top = thickness;
        if (1 == thickness)
            return QMargins();
    }
#else
    int top = thickness;
#endif

    return QMargins(thickness, top, thickness, thickness);
}

void QFramelessStyle::setWindowBorderEnabled(bool on)
{
    Q_D(QFramelessStyle);
    if (on == d->isWindowBorderEnabled)
        return;

    d->isWindowBorderEnabled = on;
    if (d->isGlobal)
    {
        for (QFramelessStyle *instance : QFramelessStylePrivate::instances)
            instance->setWindowBorderEnabled(on);
    }
    else
    {
        emit windowBorderEnabledChanged(on);
        d->updateSystemWindowBorder(true);
    }
}

void QFramelessStyle::setWindowBorderThickness(int thickness)
{
    Q_D(QFramelessStyle);
    if (thickness == d->windowBorderThickness)
        return;

    d->windowBorderThickness = thickness;
    if (d->isGlobal)
    {
        for (QFramelessStyle *instance : QFramelessStylePrivate::instances)
            instance->setWindowBorderThickness(thickness);
    }
    else
    {
        if (isWindowBorderEnabled())
            emit windowBorderThicknessChanged(thickness);
    }
}

void QFramelessStyle::setWindowBorderActiveColor(const QColor &color)
{
    Q_D(QFramelessStyle);
    if (color == d->windowBorderActiveColor)
        return;

    d->windowBorderActiveColor = color;
    if (d->isGlobal)
    {
        for (QFramelessStyle *instance : QFramelessStylePrivate::instances)
            instance->setWindowBorderActiveColor(color);
    }
    else
    {
        if (isWindowBorderEnabled())
        {
            d->updateSystemWindowBorder(false);
            emit windowBorderActiveColorChanged(color);
        }
    }
}

void QFramelessStyle::setWindowBorderInactiveColor(const QColor &color)
{
    Q_D(QFramelessStyle);
    if (color == d->windowBorderInactiveColor)
        return;

    d->windowBorderInactiveColor = color;
    if (d->isGlobal)
    {
        for (QFramelessStyle *instance : QFramelessStylePrivate::instances)
            instance->setWindowBorderInactiveColor(color);
    }
    else
    {
        if (isWindowBorderEnabled())
            emit windowBorderInactiveColorChanged(color);
    }
}

void QFramelessStyle::updateWindowBorderByActiveState()
{
   d_func()->updateSystemWindowBorder(false);
}

void QFramelessStylePrivate::updateSystemWindowBorder(bool isEnabledChanged)
{
#ifdef Q_OS_WINDOWS
    QWidget *pWindow = widget->window();
    if (!pWindow)
        return;

    Q_Q(QFramelessStyle);
    if (isEnabledChanged)
        QF::setWindowBorderVisible(pWindow->winId(), isWindowBorderEnabled);

    if (isWindowBorderEnabled)
        QF::setWindowBorderColor(pWindow->winId(), pWindow->isActiveWindow() ? q->windowBorderActiveColor() : q->windowBorderInactiveColor());
#endif
}

QFramelessStyle *QFramelessStyle::globalInstance()
{
    QFramelessStyle *pStyle = QFramelessStylePrivate::instance;
    if (!pStyle)
    {
        pStyle = new QFramelessStyle();
        QFramelessStylePrivate::instance = pStyle;
        pStyle->d_func()->isGlobal = true;
        pStyle->d_func()->refresh();
    }
    return pStyle;
}

QFramelessStyle *QFramelessStyle::instance(QWidget *widget)
{
    if (!widget)
        return nullptr;

    auto &instances = QFramelessStylePrivate::instances;
    if (!instances.contains(widget))
        instances[widget] = new QFramelessStyle(widget, globalInstance());
    return instances[widget];
}

QFramelessStyle *QFramelessStyle::instanceForWindow(QWidget *container)
{
    QWidgetsContainer *pContainer = qobject_cast<QWidgetsContainer *>(container);
    QWidget *pTarget = pContainer ? pContainer->widget() : container;
    return QFramelessStyle::instance(pTarget);
}

void QFramelessStyle::setButtonWidth(int width)
{
    Q_D(QFramelessStyle);
    if (width == d->buttonWidth)
        return;

    d->buttonWidth = width;
    emit buttonWidthChanged(width);
}

QFramelessStyle *QFramelessStylePrivate::instance = nullptr;
QHash<QWidget *, QFramelessStyle *> QFramelessStylePrivate::instances;

QFramelessStylePrivate::QFramelessStylePrivate(QFramelessStyle *q)
    : q_ptr(q)
    , titleBarContentsMargin(QF::kDefaultTitleBarContentsMargin)
    , titleBarFont(QF::systemDefaultFont())
    , windowIconSize(QF::kDefaultWindowIconSize)
    , buttonTextActiveColor(Qt::black)
    , buttonTextInactiveColor(QColor(204, 204, 204))
    , isWindowBorderEnabled(true)
    , windowBorderThickness(QF::kDefaultWindowBordeThicknessr)
    , windowBorderActiveColor(QF::kDefaultWindowBorderActiveColor)
    , windowBorderInactiveColor(QF::kDefaultWindowBorderInactiveColor)
    , widget(nullptr)
{

}

QFramelessStylePrivate::QFramelessStylePrivate(QFramelessStyle *q, QWidget *widget, QFramelessStylePrivate *target)
    : q_ptr(q)
    , titleBarContentsMargin(target->titleBarContentsMargin)
    , isTitleBarUsingQss(target->isTitleBarUsingQss)
    , titleBarFont(target->titleBarFont)
    , windowIconSize(target->windowIconSize)
    , buttonTextActiveColor(target->buttonTextActiveColor)
    , buttonTextInactiveColor(target->buttonTextInactiveColor)
    , isWindowBorderEnabled(target->isWindowBorderEnabled)
    , windowBorderThickness(target->windowBorderThickness)
    , windowBorderActiveColor(target->windowBorderActiveColor)
    , windowBorderInactiveColor(target->windowBorderInactiveColor)
    , widget(widget)
{

}

void QFramelessStylePrivate::refresh()
{
    //    const bool isColorized = QF::isTitleBarColorized();
    //    const bool isDark = (QFramelessManager::instance()->systemTheme() == QF::SystemTheme::Dark);
}
