#include "frameless/utils/qframelesshelper_p.h"

#include "frameless/utils/qframelessmanager_p.h"
#include "frameless/utils/qframelessutils_p.h"

#include <QMouseEvent>
#include <QWindow>

class QFramelessHelperUnix : public QObject
{
public:
    explicit QFramelessHelperUnix(QObject *parent);
    ~QFramelessHelperUnix() override = default;

    const QObject *window { };

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
};

struct QFramelessDataUnix : public QFramelessData
{
    QFramelessHelperUnix *m_pHelper { };
    bool isCursorShapeChanged { };
    bool isLeftButtonPressed { };
};
using QFramelessDataUnixPtr = std::shared_ptr<QFramelessDataUnix>;

QFramelessData::PtrType QFramelessData::create()
{
    return std::make_shared<QFramelessDataUnix>();
}

static inline QFramelessDataUnixPtr tryGetData(const QObject *window)
{
    Q_ASSERT(window);
    if (!window)
        return nullptr;

    QFramelessDataPtr data = QFramelessManager::data(window);
    if (!data)
        return nullptr;
    return std::dynamic_pointer_cast<QFramelessDataUnix>(data);
}

void QFramelessHelperPrivate::initQtGlobalAttributes()
{

}

void QFramelessHelperPrivate::showSystemMenu(const QPoint &point)
{
    if (!window)
        return;

    const WId windowId = window->winId();
    const QPoint nativePos = QF::toNativeGlobalPosition(window->windowHandle(), point);
    QF::showSystemMenu(windowId, nativePos);
}

void QFramelessHelperPrivate::setupFramelessWindow(const QObject* window)
{
    Q_ASSERT(window);
    if (!window)
        return;

    const QFramelessDataUnixPtr data = tryGetData(window);
    if (!data || data->isFrameless || !data->callbacks)
        return;

    QWindow *qWindow = data->callbacks->getWindowHandle();
    Q_ASSERT(qWindow);
    if (!qWindow)
        return;

    data->callbacks->setWindowFlags(data->callbacks->getWindowFlags() | Qt::FramelessWindowHint);

    data->isFrameless = true;
    if (!data->m_pHelper)
    {
        data->m_pHelper = new QFramelessHelperUnix(qWindow);
        data->m_pHelper->window = window;
        qWindow->installEventFilter(data->m_pHelper);
    }
}

void QFramelessHelperPrivate::teardownFramelessWindow(const QObject* window)
{
    (void)window;
}

QFramelessHelperUnix::QFramelessHelperUnix(QObject *parent)
    : QObject(parent)
{

}

bool QFramelessHelperUnix::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (!object || !event)
        return false;

    if (!window || !object->isWindowType())
        return false;

    const QFramelessDataUnixPtr data = tryGetData(window);
    if (!data || !data->isFrameless || !data->callbacks)
        return false;

    const QEvent::Type type = event->type();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))
    if (type == QEvent::DevicePixelRatioChange)
#else
    if (type == QEvent::ScreenChangeInternal)
#endif
    {
        data->callbacks->forceChildrenRepaint();
        return false;
    }

    const auto mouseEvent = static_cast<QMouseEvent *>(event);
    if (!mouseEvent)
        return false;

    const auto qWindow = qobject_cast<QWindow *>(object);
    const Qt::MouseButton button = mouseEvent->button();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QPoint scenePos = mouseEvent->scenePosition().toPoint();
    const QPoint globalPos = mouseEvent->globalPosition().toPoint();
#else
    const QPoint scenePos = mouseEvent->windowPos().toPoint();
    const QPoint globalPos = mouseEvent->screenPos().toPoint();
#endif
    const bool isWindowFixedSize = data->callbacks->isWindowFixedSize();
    const bool isInsideTitleBar = data->callbacks->isInsideTitleBarDraggableArea(scenePos);
    const bool isIgnoreThisEvent = data->callbacks->isShouldIgnoreMouseEvents(scenePos);

    switch (type)
    {
    case QEvent::MouseButtonPress:
        if (Qt::LeftButton == button)
        {
            data->isLeftButtonPressed = true;
        }
        break;
    case QEvent::MouseButtonRelease:
        if (Qt::LeftButton == button)
            data->isLeftButtonPressed = false;
        else if (Qt::RightButton == button)
        {
            if (!isIgnoreThisEvent && isInsideTitleBar)
            {
                data->callbacks->showSystemMenu(globalPos);
                return true;
            }
        }
        break;
    case QEvent::MouseButtonDblClick:
        if (Qt::LeftButton == button && !isWindowFixedSize && !isIgnoreThisEvent && isInsideTitleBar)
        {
            Qt::WindowState newWinState = Qt::WindowNoState;
            if (data->callbacks->getWindowState() != Qt::WindowMaximized)
                newWinState = Qt::WindowMaximized;
            data->callbacks->setWindowState(newWinState);
            return true;
        }
        break;
    case QEvent::MouseMove:
        if (!isWindowFixedSize && !data->isLeftButtonPressed)
        {
            const Qt::CursorShape cs = QF::calculateCursorShape(qWindow, scenePos);
            if (Qt::ArrowCursor == cs)
            {
                if (data->isCursorShapeChanged)
                {
                    data->callbacks->unsetCursor();
                    data->isCursorShapeChanged = false;
                }
                else
                {
                    data->callbacks->setCursor(cs);
                    data->isCursorShapeChanged = true;
                }
            }
        }

        if (data->isLeftButtonPressed)
        {
            if (!isIgnoreThisEvent && isInsideTitleBar)
            {
                QF::startSystemMove(qWindow, globalPos);
                return true;
            }
        }
        break;
    default:
        break;
    }

    return false;
}
