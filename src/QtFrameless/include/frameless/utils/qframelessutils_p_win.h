#ifndef QFRAMELESSUTILS_P_WIN_H
#define QFRAMELESSUTILS_P_WIN_H

#include <Windows.h>
#include "qframelessglobal_p.h"

QF_BEGIN_NAMESPACE

enum class DwmColorizationArea : quint8
{
    None,
    StartMenu_TaskBar_ActionCenter,
    TitleBar_WindowBorder,
    All
};

bool isValidWindow(const WId windowId, const bool checkVisible, const bool checkTopLevel);
bool isWindowFrameBorderVisible();
bool isDwmCompositionEnabled();
bool isFullScreen(const WId windowId);
bool isWindowNoState(const WId windowId);

void setWindowBorderColor(const WId windowId, const QColor &color);
void setWindowBorderVisible(const WId windowId, const bool isVisible);

quint32 getTitleBarHeight(const WId windowId, const bool scaled);
quint32 getCaptionBarHeight(const WId windowId, const bool scaled);
quint32 getResizeBorderThickness(const WId windowId, const bool horizontal, const bool scaled);

int getSystemMetricsForDpi(const int nIndex, const quint32 dpi);
int getSystemMetricsForDpi(const int index, const quint32 dpi, const bool horizontal);
int getSystemMetrics(const WId windowId, const int index, const bool horizontal, const bool scaled);

DwmColorizationArea getDwmColorizationArea();

quint32 getWindowDpi(const HWND hWnd);
quint32 getWindowDpi(const WId windowId, const bool isHor);
quint32 getPrimaryScreenDpi(const bool isHor);
bool tryToEnableHighestDpiAwarenessLevel();
bool fixupChildWindowsDpiMessage(const WId windowId);
bool fixupDialogsDpiScaling();
bool setChildWindowDpiMessageEnabled(const HWND hWnd, const bool on);

quint64 getKeyState();
bool getMonitorForWindow(const HWND hwnd, MONITORINFOEXW& info);
QMargins getWindowSystemFrameMargins(const WId windowId);
QMargins getWindowCustomFrameMargins(const QWindow* window);

bool setWindowThemeAttribute(const HWND hWnd, void* pvData, const quint32 cbData);
bool setWindowThemeNonClientAttributes(const HWND hWnd, const quint32 dwMask, const quint32 dwAttributes);

bool triggerFrameChange(const WId windowId);
bool updateInternalWindowFrameMargins(QWindow* window, const bool enable);
bool updateAllDirectXSurfaces();
bool hideOriginalTitleBarElements(const WId windowId, const bool disable);
bool syncWmPaintWithDwm();

QF_END_NAMESPACE

#endif // QFRAMELESSUTILS_P_WIN_H
