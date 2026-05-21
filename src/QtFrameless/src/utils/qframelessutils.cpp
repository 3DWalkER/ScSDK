#include "frameless/utils/qframelessutils_p.h"

#include "frameless/utils/qframelessstyle_p.h"

#include <QHash>
#include <QtGui/private/qhighdpiscaling_p.h>


QF_BEGIN_NAMESPACE

QPoint toNativePixels(const QWindow *window, const QPoint &point)
{
    Q_ASSERT(window);
    if (!window)
        return { };
    return QHighDpi::toNativePixels(point, window);
}

QSize toNativePixels(const QWindow* window, const QSize& size)
{
    Q_ASSERT(window);
    if (!window)
        return { };
    return QHighDpi::toNativePixels(size, window);
}

QPoint toNativeGlobalPosition(const QWindow *window, const QPoint &point)
{
    Q_ASSERT(window);
    if (!window)
        return { };

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    return toNativePixels(window, point);
#else
    return QHighDpi::toNativeGlobalPosition(point, window);
#endif
}

QPoint fromNativeLocalPosition(const QWindow *window, const QPoint &point)
{
    Q_ASSERT(window);
    if (!window)
        return { };
    return QHighDpi::fromNativeLocalPosition(point, window);
}

Qt::WindowState windowStatesToWindowState(const Qt::WindowStates states)
{
    if (states & Qt::WindowFullScreen)
        return Qt::WindowFullScreen;

    if (states & Qt::WindowMaximized)
        return Qt::WindowMaximized;

    if (states & Qt::WindowMinimized)
        return Qt::WindowMinimized;

    return Qt::WindowNoState;
}

Qt::CursorShape calculateCursorShape(const QWindow *window, const QPoint &pos)
{
#ifdef Q_OS_MACOS
    Q_UNUSED(window);
    Q_UNUSED(pos);
    return Qt::ArrowCursor;
#else
    Q_ASSERT(window);
    if (!window)
        return Qt::ArrowCursor;

    if (window->visibility() != QWindow::Windowed)
        return Qt::ArrowCursor;

    const int x = pos.x();
    const int y = pos.y();
    const int w = window->width();
    const int h = window->height();
    if (((x < QF::kDefaultResizeBorderThickness) && (y < QF::kDefaultResizeBorderThickness))
            || ((x >= (w - QF::kDefaultResizeBorderThickness)) && (y >= (h - QF::kDefaultResizeBorderThickness))))
        return Qt::SizeFDiagCursor;

    if (((x >= (w - QF::kDefaultResizeBorderThickness)) && (y < QF::kDefaultResizeBorderThickness))
            || ((x < QF::kDefaultResizeBorderThickness) && (y >= (h - QF::kDefaultResizeBorderThickness))))
        return Qt::SizeBDiagCursor;

    if ((x < QF::kDefaultResizeBorderThickness) || (x >= (w - QF::kDefaultResizeBorderThickness)))
        return Qt::SizeHorCursor;

    if ((y < QF::kDefaultResizeBorderThickness) || (y >= (h - QF::kDefaultResizeBorderThickness)))
        return Qt::SizeVerCursor;
    return Qt::ArrowCursor;
#endif
}

QF_END_NAMESPACE
