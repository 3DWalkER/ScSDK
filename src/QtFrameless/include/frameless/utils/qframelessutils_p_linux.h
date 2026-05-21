#ifndef QFRAMELESSUTILS_P_LINUX_H
#define QFRAMELESSUTILS_P_LINUX_H

#include "qframelessglobal_p.h"

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    using x11_return_type = quint32;
#else   // (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    using x11_return_type = unsigned long;
#endif  // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

using xcb_connection_t = struct xcb_connection_t;
using xcb_button_t = uint8_t;
using xcb_window_t = uint32_t;
using xcb_timestamp_t = uint32_t;
using xcb_atom_t = uint32_t;

QF_BEGIN_NAMESPACE

xcb_connection_t *x11_connection();
x11_return_type x11_appRootWindow(const int screen);
QScreen *x11_findScreenForVirtualDesktop(const int virtualDesktopNumber);
int x11_appScreen();
xcb_atom_t internAtom(const char *name);

bool isSupportedByWindowManager(const xcb_atom_t atom);
bool isCustomDecorationSupported();

void showSystemMenu(const WId windowId, const QPoint &globalPos);

QF_END_NAMESPACE

#endif // QFRAMELESSUTILS_P_LINUX_H
