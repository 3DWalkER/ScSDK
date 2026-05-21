#ifndef QFRAMELESSUTILS_H
#define QFRAMELESSUTILS_H

#include <qcompilerdetection.h>
#if defined(Q_OS_WINDOWS)
#   include "qframelessutils_p_win.h"
#elif defined(Q_OS_LINUX)
#   include "qframelessutils_p_linux.h"
#endif

QF_BEGIN_NAMESPACE

bool startSystemMove(QWindow *window, const QPoint &globalPos);

bool isTitleBarColorized();
bool isWindowAttachedToEdge(const WId windowId);

QPoint toNativePixels(const QWindow *window, const QPoint &point);
QSize toNativePixels(const QWindow* window, const QSize& size);
QPoint toNativeGlobalPosition(const QWindow *window, const QPoint &point);
QPoint fromNativeLocalPosition(const QWindow* window, const QPoint& point);
Qt::WindowState windowStatesToWindowState(const Qt::WindowStates states);

Qt::CursorShape calculateCursorShape(const QWindow *window, const QPoint &pos);

bool shouldAppsUseDarkMode();
QColor accentColor();
QFont systemDefaultFont();
int windowCornerRadius(const WId windowId);

QF_END_NAMESPACE

#endif // QFRAMELESSUTILS_H
