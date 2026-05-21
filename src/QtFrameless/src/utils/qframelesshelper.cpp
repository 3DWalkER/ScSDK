#include "frameless/utils/qframelesshelper_p.h"

#include "frameless/utils/qframelessmanager_p.h"
#include "frameless/utils/qframelessutils_p.h"
#include "frameless/utils/qframelessstyle_p.h"

#include <QApplication>
#include <QGuiApplication>
#include <QPaintEvent>
#include <QWidget>
#include <utility>
#include <QDebug>

extern Q_WIDGETS_EXPORT QWidget *qt_button_down;

/**
 * @brief The QFramelessTitleBarData struct 无边框标题栏信息
 */
struct QFramelessTitleBarData : public QFramelessExtraData
{
    QPointer<QWidget> titleBarWidget{ };
    QList<QPointer<QWidget>> hitTestVisibleWidgets{ };
    QPointer<QWidget> windowIconButton{ };
    QPointer<QWidget> contextHelpButton{ };
    QPointer<QWidget> minimizeButton{ };
    QPointer<QWidget> maximizeButton{ };
    QPointer<QWidget> closeButton{ };
    QList<QRect> hitTestVisibleRects{ };

    ~QFramelessTitleBarData() override { }

    static QFramelessExtraDataPtr create() { return std::make_shared<QFramelessTitleBarData>(); }
};
using QFramelessTitleBarDataPtr = std::shared_ptr<QFramelessTitleBarData>;

static inline QFramelessTitleBarDataPtr tryGetExtraData(const QFramelessDataPtr &data, bool create)
{
    Q_ASSERT(data);
    if (!data)
        return nullptr;

    auto it = data->extraData.find(QF::ExtraDataType::FramelessWidgetsHelper);
    if (data->extraData.end() == it)
    {
        if (create)
            it = data->extraData.insert(QF::ExtraDataType::FramelessWidgetsHelper, QFramelessTitleBarData::create());
        else
            return nullptr;
    }
    return std::dynamic_pointer_cast<QFramelessTitleBarData>(it.value());
}

static inline QFramelessTitleBarDataPtr tryGetExtraData(const QWidget *window, bool create)
{
    Q_ASSERT(window);
    if (!window)
        return nullptr;

    const QFramelessDataPtr data = QFramelessManager::data(window);
    return !data ? nullptr : tryGetExtraData(data, create);
}

QFramelessHelper::QFramelessHelper(QObject *parent)
    : QObject(parent)
    , d_ptr(new QFramelessHelperPrivate(this))
{
    Q_D(QFramelessHelper);
    connect(&d->repaintTimer, SIGNAL(timeout()), this, SLOT(doRepaintAllChildren()));
}

QFramelessHelper::~QFramelessHelper()
{
    delete d_ptr;
}

bool QFramelessHelper::isExtended() const
{
    Q_D(const QFramelessHelper);
    if (!d->window)
        return false;

    const QFramelessDataPtr data = QFramelessManager::data(d->window);
    return data && data->isFrameless;
}

bool QFramelessHelper::isWindowFixedSize() const
{
    Q_D(const QFramelessHelper);
    return !d->window ? false : isWidgetFixedSize(d->window);
}

void QFramelessHelper::moveWindowToDesktopCenter()
{
    Q_D(QFramelessHelper);
    if (!d->window)
        return;
    //    QFramelessManager::moveWindowToDesktopCenter(d->window->winId(), true);
}

void QFramelessHelper::setHitTestVisible(QWidget *widget, bool on)
{
    Q_ASSERT(widget);
    if (!widget)
        return;

    Q_D(QFramelessHelper);
    if (!d->window)
        return;

    QFramelessTitleBarDataPtr titleBarData = tryGetExtraData(d->window, false);
    Q_ASSERT(titleBarData);
    if (!titleBarData)
        return;

    if (on)
        titleBarData->hitTestVisibleWidgets.append(widget);
    else
        titleBarData->hitTestVisibleWidgets.removeAll(widget);
}

void QFramelessHelper::initQtGlobalAttributes()
{
    static bool inited = false;
    if (inited)
        return;
    inited = true;

    QFramelessHelperPrivate::initQtGlobalAttributes();

    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
}

bool QFramelessHelper::isWidgetFixedSize(const QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget)
        return false;

    if (widget->windowFlags() & Qt::MSWindowsFixedSizeDialogHint)
        return true;

    const QSize minSize = widget->minimumSize();
    const QSize maxSize = widget->maximumSize();
    if (!minSize.isEmpty() && !maxSize.isEmpty() && (minSize == maxSize))
        return true;

    const QSizePolicy sizePolicy = widget->sizePolicy();
    if ((sizePolicy.horizontalPolicy() == QSizePolicy::Fixed) && (sizePolicy.verticalPolicy() == QSizePolicy::Fixed))
        return true;
    return false;
}

QFramelessHelper *QFramelessHelper::instance(QObject *obj)
{
    Q_ASSERT(obj);
    if (!obj)
        return nullptr;
    return QFramelessHelperPrivate::findOrCreateFramelessHelper(obj);
}

void QFramelessHelper::extends(bool on)
{
    if (isExtended() == on)
        return;

    Q_D(QFramelessHelper);
    on ? d->attach() : d->detach();
}

void QFramelessHelper::setTitleBarWidget(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget)
        return;

    Q_D(QFramelessHelper);
    if (!d->window)
        return;

    QFramelessTitleBarDataPtr titleBarData = tryGetExtraData(d->window, false);
    Q_ASSERT(titleBarData);
    if (!titleBarData || widget == titleBarData->titleBarWidget)
        return;

    titleBarData->titleBarWidget = widget;
}

void QFramelessHelper::setSystemButton(QWidget *widget, QSystemButton::Type type)
{
    Q_ASSERT(widget);
    Q_ASSERT(QSystemButton::Invalid != type);
    if (!widget || QSystemButton::Invalid == type)
        return;

    Q_D(QFramelessHelper);
    if (!d->window)
        return;

    QFramelessTitleBarDataPtr titleBarData = tryGetExtraData(d->window, false);
    Q_ASSERT(titleBarData);
    if (!titleBarData)
        return;

    switch (type)
    {
    case QSystemButton::WindowIcon:
        titleBarData->windowIconButton = widget;
        break;
    case QSystemButton::Help:
        titleBarData->contextHelpButton = widget;
        break;
    case QSystemButton::Minimize:
        titleBarData->minimizeButton = widget;
        break;
    case QSystemButton::Maximize:
    case QSystemButton::Restore:
        titleBarData->maximizeButton = widget;
        break;
    case QSystemButton::Close:
        titleBarData->closeButton = widget;
        break;
    default:
        Q_UNREACHABLE();
    }
}

QFramelessHelperPrivate::QFramelessHelperPrivate(QFramelessHelper *q)
    : q_ptr(q)
{
    repaintTimer.setTimerType(Qt::VeryCoarseTimer);
    repaintTimer.setInterval(10);
}

QFramelessHelperPrivate::~QFramelessHelperPrivate()
{
    detach();
}

void QFramelessHelperPrivate::attach()
{
    QWidget *tlw = findTopLevelWindow();
    Q_ASSERT(tlw);
    if (nullptr == tlw || tlw == window)
        return;

    window = tlw;
    if (!window->testAttribute(Qt::WA_DontCreateNativeAncestors))
        window->setAttribute(Qt::WA_DontCreateNativeAncestors);

    if (!window->testAttribute(Qt::WA_NativeWindow))
        window->setAttribute(Qt::WA_NativeWindow);

    const WId windowId = window->winId();
    const QFramelessDataPtr data = QFramelessManager::create(window, windowId);
    Q_ASSERT(data);
    if (!data || data->isFrameless)
        return;

    Q_Q(QFramelessHelper);
    if (!data->callbacks)
    {
        data->callbacks = QFramelessCallbacks::create();
        data->callbacks->getWindowFlags = [this]() -> Qt::WindowFlags { return window->windowFlags(); };
        data->callbacks->setWindowFlags = [this](const Qt::WindowFlags flags) -> void { window->setWindowFlags(flags); };
        data->callbacks->getWindowId = [this]() -> WId { return window->winId(); };
        data->callbacks->getWindowHandle = [this]() -> QWindow * { return window->windowHandle(); };
        data->callbacks->getWindowSize = [this]() -> QSize { return window->size(); };
        data->callbacks->getWindowState = [this]() -> Qt::WindowState { return QF::windowStatesToWindowState(window->windowState()); };
        data->callbacks->setWindowState = [this](const Qt::WindowState state) -> void { window->setWindowState(state); };
        data->callbacks->isWindowFixedSize = [q]() -> bool { return q->isWindowFixedSize(); };
        data->callbacks->setWindowPosition = [this](const QPoint &pos) -> void { window->move(pos); };
        data->callbacks->getWindowScreen = [this]() -> QScreen * {
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
                return window->screen();
        #else
                return window->windowHandle()->screen();
        #endif
    };
        data->callbacks->isInsideTitleBarDraggableArea = [this](const QPoint &pos) -> bool { return isInTitleBarDraggableArea(pos); };
        data->callbacks->isInsideSystemButtons = [this](const QPoint &pos, QSystemButton::Type *button) -> bool { return isInSystemButton(pos, button); };
        data->callbacks->isShouldIgnoreMouseEvents = [this](const QPoint &pos) -> bool { return isShouldIgnoreMouseEvents(pos); };
        data->callbacks->setProperty = [this](const char *name, const QVariant &value) -> void { setProperty(name, value); };
        data->callbacks->getProperty = [this](const char *name, const QVariant &defaultValue) -> QVariant { return property(name, defaultValue); };
        data->callbacks->forceChildrenRepaint = [this]() -> void { repaintAllChildren(); };
        data->callbacks->resetQtGrabbedControl = []() -> bool {
            if (qt_button_down) {
                static constexpr const auto invalidPos = QPoint{ -99999, -99999 };
                const auto event = new QMouseEvent(
                            QEvent::MouseButtonRelease,
                            invalidPos,
                            invalidPos,
                            invalidPos,
                            Qt::LeftButton,
                            QGuiApplication::mouseButtons() ^ Qt::LeftButton,
                            QGuiApplication::keyboardModifiers());
                QApplication::postEvent(qt_button_down, event);
                qt_button_down = nullptr;
                return true;
            }
            return false;
        };
        data->callbacks->showSystemMenu = [this](const QPoint &pos) { showSystemMenu(pos); };
        data->callbacks->setCursor = [this](const QCursor &cursor) -> void { window->setCursor(cursor); };
        data->callbacks->unsetCursor = [this]() -> void { window->unsetCursor(); };
    }
    
    tryGetExtraData(data, true);
    if (QFramelessManager::instance()->addWindow(window, windowId))
        setupFramelessWindow(window);

    q->moveWindowToDesktopCenter();
}

void QFramelessHelperPrivate::detach()
{
    if (!window)
        return;

    QFramelessManager *pMgr = QFramelessManager::instance();
    QFramelessDataPtr pData = pMgr->data(window);
    if (nullptr != pData)
    {
        teardownFramelessWindow(window);
        pMgr->removeWindow(window);
    }
    window = nullptr;
}

void QFramelessHelperPrivate::setProperty(const char *name, const QVariant &value)
{
    Q_ASSERT(name);
    Q_ASSERT(*name != '\0');
    Q_ASSERT(value.isValid());
    Q_ASSERT(!value.isNull());
    if (!name || (*name == '\0') || !value.isValid() || value.isNull())
        return;

    Q_ASSERT(window);
    if (window)
        window->setProperty(name, value);
}

QVariant QFramelessHelperPrivate::property(const char *name, const QVariant &defaultValue)
{
    Q_ASSERT(name);
    Q_ASSERT(*name != '\0');
    if (!name || (*name == '\0'))
        return defaultValue;

    Q_ASSERT(window);
    if (window)
        return defaultValue;

    const QVariant value = window->property(name);
    return ((value.isValid() && !value.isNull()) ? value : defaultValue);
}

QWidget *QFramelessHelperPrivate::findTopLevelWindow() const
{
    Q_Q(const QFramelessHelper);
    const QObject *const p = q->parent();
    Q_ASSERT(p);
    if (p)
    {
        if (const auto parentWidget = qobject_cast<const QWidget *>(p))
            return parentWidget->window();
    }
    return nullptr;
}

QRect QFramelessHelperPrivate::mapWidgetGeometryToScene(const QWidget * const widget) const
{
    Q_ASSERT(widget);
    if (!widget)
        return { };

    if (!window)
        return { };

    const QPoint originPoint = widget->mapTo(window, QPoint(0, 0));
    const QSize size = widget->size();
    return QRect(originPoint, size);
}

bool QFramelessHelperPrivate::isInTitleBarDraggableArea(const QPoint &pos) const
{
    if (!window)
        return false;

    const QFramelessTitleBarDataPtr data = tryGetExtraData(window, false);
    Q_ASSERT(data);
    if (!data)
        return false;

    if (!data->titleBarWidget || !data->titleBarWidget->isVisible() || !data->titleBarWidget->isEnabled())
        return false;

    const QRect windowRect = { QPoint(0, 0), window->size() };
    const QRect titleBarRect = mapWidgetGeometryToScene(data->titleBarWidget);
    if (!titleBarRect.intersects(windowRect))
        return false;

    QRegion region = titleBarRect;
    QVector<QWidget *> systemButtons = {
        data->windowIconButton, data->contextHelpButton,
        data->minimizeButton,   data->maximizeButton,
        data->closeButton
    };

    for (QWidget *&button : systemButtons)
    {
        if (button && button->isVisible() && button->isEnabled())
            region -= mapWidgetGeometryToScene(button);
    }

    if (!data->hitTestVisibleWidgets.isEmpty())
    {
        for (QPointer<QWidget> &widget : data->hitTestVisibleWidgets)
        {
            if (widget && widget->isVisible() && widget->isEnabled())
                region -= mapWidgetGeometryToScene(widget);
        }
    }

    if (!data->hitTestVisibleRects.isEmpty())
    {
        for (const QRect &rect : data->hitTestVisibleRects)
        {
            if (rect.isValid())
                region -= rect;
        }
    }
    return region.contains(pos);
}

bool QFramelessHelperPrivate::isInSystemButton(const QPoint &pos, QSystemButton::Type *button) const
{
    Q_ASSERT(button);
    if (!button)
        return false;

    if (!window)
        return false;

    const QFramelessTitleBarDataPtr data = tryGetExtraData(window, false);
    Q_ASSERT(data);
    if (!data)
        return false;

    *button = QSystemButton::Invalid;
    if (data->windowIconButton && data->windowIconButton->isVisible() && data->windowIconButton->isEnabled())
    {
        if (mapWidgetGeometryToScene(data->windowIconButton).contains(pos))
        {
            *button = QSystemButton::WindowIcon;
            return true;
        }
    }

    if (data->contextHelpButton && data->contextHelpButton->isVisible() && data->contextHelpButton->isEnabled())
    {
        if (mapWidgetGeometryToScene(data->contextHelpButton).contains(pos))
        {
            *button = QSystemButton::Help;
            return true;
        }
    }

    if (data->minimizeButton && data->minimizeButton->isVisible() && data->minimizeButton->isEnabled())
    {
        if (mapWidgetGeometryToScene(data->minimizeButton).contains(pos))
        {
            *button = QSystemButton::Minimize;
            return true;
        }
    }

    if (data->maximizeButton && data->maximizeButton->isVisible() && data->maximizeButton->isEnabled())
    {
        if (mapWidgetGeometryToScene(data->maximizeButton).contains(pos))
        {
            *button = QSystemButton::Maximize;
            return true;
        }
    }

    if (data->closeButton && data->closeButton->isVisible() && data->closeButton->isEnabled())
    {
        if (mapWidgetGeometryToScene(data->closeButton).contains(pos))
        {
            *button = QSystemButton::Close;
            return true;
        }
    }

    return false;
}

bool QFramelessHelperPrivate::isShouldIgnoreMouseEvents(const QPoint &pos) const
{
    if (!window)
        return false;

    const auto isWinthinFrameBorder = [this, &pos]() ->bool {
        if (QFramelessHelper::isWidgetFixedSize(window))
            return false;

        if (pos.y() < QF::kDefaultResizeBorderThickness)
            return true;

#ifdef Q_OS_WINDOWS
        if (QF::isWindowFrameBorderVisible())
            return false;
#endif
        return ((pos.x() < QF::kDefaultResizeBorderThickness)
                || (pos.x() >= (window->width() - QF::kDefaultResizeBorderThickness)));
    }();

    return (QF::windowStatesToWindowState(window->windowState()) == Qt::WindowNoState) && isWinthinFrameBorder;
}

void QFramelessHelperPrivate::repaintAllChildren()
{
    repaintTimer.start();
}

void QFramelessHelperPrivate::doRepaintAllChildren()
{
    repaintTimer.stop();
    if (!window)
        return;

    forceWidgetRepaint(window);
    QList<QWidget *> widgets = window->findChildren<QWidget *>();
    for (QWidget *&widget : widgets)
        forceWidgetRepaint(widget);
}

void QFramelessHelperPrivate::forceWidgetRepaint(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget)
        return;

#if (defined(Q_OS_WINDOWS) && (QT_VERSION != QT_VERSION_CHECK(6, 5, 3)) && (QT_VERSION != QT_VERSION_CHECK(6, 6, 0)))
    if (widget->isWindow())
    {
        if (QWindow * const window = widget->windowHandle())
            QF::updateInternalWindowFrameMargins(window, true);
    }
#endif

#if 0
    if (!widget->isVisible())
        return;

    widget->update();
    if (!widget->isWindow() || !(widget->windowState() & (Qt::WindowMinimized | Qt::WindowMaximized | Qt::WindowFullScreen)))
    {
        if (!isWidgetFixedSize(widget))
        {
            const QSize originalSize = widget->size();
            static constexpr const auto margins = QMargins{ 1, 1, 1, 1 };
            widget->resize(originalSize.shrunkBy(margins));
            widget->resize(originalSize.grownBy(margins));
            widget->resize(originalSize);
        }

        const QPoint originalPosition = widget->pos();
        static constexpr const auto offset = QPoint{ 10, 10 };
        widget->move(originalPosition - offset);
        widget->move(originalPosition + offset);
        widget->move(originalPosition);
    }
    widget->update();
#endif
}

QFramelessHelper *QFramelessHelperPrivate::findOrCreateFramelessHelper(QObject *obj)
{
    Q_ASSERT(obj);
    if (!obj)
        return nullptr;

    QObject *parent = nullptr;
    if (const auto widget = qobject_cast<QWidget *>(obj))
        parent = widget->window();
    else
        parent = obj;

    QFramelessHelper *instance = parent->findChild<QFramelessHelper *>();
    if (!instance)
    {
        instance = new QFramelessHelper(parent);
    }
    return instance;
}

#include "frameless/utils/moc_qframelesshelper.cpp"
