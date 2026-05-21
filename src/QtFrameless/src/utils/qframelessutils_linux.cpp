#include "frameless/utils/qframelessutils_linux_p.h"
#include "frameless/utils/qframelessutils_p.h"

#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/qpa/qplatformwindow.h>
#include <qcompilerdetection.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <QMouseEvent>
#include <string.h>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#   include <QtGui/qpa/qplatformscreen_p.h>
#   include <QtGui/qpa/qplatformscreen.h>
#else
#   include <QtPlatformHeaders/qxcbscreenfunctions.h>
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
#   include <QtGui/private/qtx11extras_p.h>
#   define QF_HAS_X11EXTRAS
#endif

constexpr const auto _NET_WM_MOVERESIZE_SIZE_TOPLEFT = 0;
constexpr const auto _NET_WM_MOVERESIZE_SIZE_TOP = 1;
constexpr const auto _NET_WM_MOVERESIZE_SIZE_TOPRIGHT = 2;
constexpr const auto _NET_WM_MOVERESIZE_SIZE_RIGHT = 3;
constexpr const auto _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT = 4;
constexpr const auto _NET_WM_MOVERESIZE_SIZE_BOTTOM = 5;
constexpr const auto _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT = 6;
constexpr const auto _NET_WM_MOVERESIZE_SIZE_LEFT = 7;
constexpr const auto _NET_WM_MOVERESIZE_MOVE = 8;
constexpr const auto _NET_WM_MOVERESIZE_SIZE_KEYBOARD = 9;
constexpr const auto _NET_WM_MOVERESIZE_MOVE_KEYBOARD = 10;
constexpr const auto _NET_WM_MOVERESIZE_CANCEL = 11;

static constexpr const auto _XCB_SEND_EVENT_MASK =
    (XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY);

constexpr const char ATOM_NET_SUPPORTED[] = "_NET_SUPPORTED";
constexpr const char ATOM_NET_WM_NAME[] = "_NET_WM_NAME";
constexpr const char ATOM_NET_WM_MOVERESIZE[] = "_NET_WM_MOVERESIZE";
constexpr const char ATOM_NET_SUPPORTING_WM_CHECK[] = "_NET_SUPPORTING_WM_CHECK";
constexpr const char ATOM_NET_KDE_COMPOSITE_TOGGLING[] = "_NET_KDE_COMPOSITE_TOGGLING";
constexpr const char ATOM_KDE_NET_WM_BLUR_BEHIND_REGION[] = "_KDE_NET_WM_BLUR_BEHIND_REGION";
constexpr const char ATOM_GTK_SHOW_WINDOW_MENU[] = "_GTK_SHOW_WINDOW_MENU";
constexpr const char ATOM_DEEPIN_NO_TITLEBAR[] = "_DEEPIN_NO_TITLEBAR";
constexpr const char ATOM_DEEPIN_FORCE_DECORATE[] = "_DEEPIN_FORCE_DECORATE";
constexpr const char ATOM_NET_WM_DEEPIN_BLUR_REGION_MASK[] = "_NET_WM_DEEPIN_BLUR_REGION_MASK";
constexpr const char ATOM_NET_WM_DEEPIN_BLUR_REGION_ROUNDED[] = "_NET_WM_DEEPIN_BLUR_REGION_ROUNDED";
constexpr const char ATOM_UTF8_STRING[] = "UTF8_STRING";

QF_BEGIN_NAMESPACE

static inline void generateMouseReleaseEvent(QWindow *window, const QPoint &globalPos)
{
    Q_ASSERT(window);
    if (!window)
        return;

    const QPoint localPos = window->mapFromGlobal(globalPos);
    const QPoint scenePos = localPos; // windowPos in Qt5.
    const auto event = new QMouseEvent(
        QEvent::MouseButtonRelease,
        localPos,
        scenePos,
        globalPos,
        Qt::LeftButton,
        QGuiApplication::mouseButtons() ^ Qt::LeftButton,
        QGuiApplication::keyboardModifiers());
    QGuiApplication::postEvent(window, event);
}

static void sendMoveResizeMessage(const WId windowId, const uint32_t action, const QPoint &globalPos, const Qt::MouseButton button = Qt::LeftButton)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return;

    xcb_connection_t * const connection = x11_connection();
    Q_ASSERT(connection);
    if (!connection)
        return;

    const quint32 rootWindow = x11_appRootWindow(x11_appScreen());
    Q_ASSERT(rootWindow);
    if (!rootWindow)
        return;

    static const xcb_atom_t atom = internAtom(ATOM_NET_WM_MOVERESIZE);
    if ((atom == XCB_NONE) || !isSupportedByWindowManager(atom))
        return;

    xcb_client_message_event_t xev;
    memset(&xev, 0, sizeof(xev));
    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = atom;
    xev.window = windowId;
    xev.format = 32;
    xev.data.data32[0] = globalPos.x();
    xev.data.data32[1] = globalPos.y();
    xev.data.data32[2] = action;
    xev.data.data32[3] = [button]() -> int {
        if (button == Qt::LeftButton) {
            return XCB_BUTTON_INDEX_1;
        }
        if (button == Qt::RightButton) {
            return XCB_BUTTON_INDEX_3;
        }
        return XCB_BUTTON_INDEX_ANY;
    }();
    xev.data.data32[4] = 0;

    if (action != _NET_WM_MOVERESIZE_CANCEL)
        xcb_ungrab_pointer(connection, XCB_CURRENT_TIME);
    xcb_send_event(connection, false, rootWindow, _XCB_SEND_EVENT_MASK, reinterpret_cast<const char *>(&xev));
    xcb_flush(connection);
}

bool startSystemMove(QWindow *window, const QPoint &globalPos)
{
    Q_ASSERT(window);
    if (!window)
        return false;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    window->startSystemMove();
    generateMouseReleaseEvent(window, globalPos);
    return true;
#else   // (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    const QPoint nativeGlobalPos = Utils::toNativeGlobalPosition(window, globalPos);
    sendMoveResizeMessage(window->winId(), _NET_WM_MOVERESIZE_MOVE, nativeGlobalPos);
    return true;
#endif
}

bool isTitleBarColorized()
{
    return false;
}

bool shouldAppsUseDarkMode()
{
    return false;
}

QColor accentColor()
{
    return QColor();
}

QFont systemDefaultFont()
{
    return QFont();
}

xcb_connection_t *x11_connection()
{
#ifdef QF_HAS_X11EXTRAS
    return QX11Info::connection();
#endif

    if (!qApp)
        return nullptr;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    using App = QNativeInterface::QX11Application;
    const auto native = qApp->nativeInterface<App>();
#else   // (QT_VERSION < QT_VERSION_CHECK(6, 2, 0))
    const auto native = qApp->platformNativeInterface();
#endif  // (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    if (!native)
        return nullptr;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
    return native->connection();
#else   // (QT_VERSION < QT_VERSION_CHECK(6, 2, 0))
    return reinterpret_cast<xcb_connection_t *>(native->nativeResourceForIntegration("connection"));
#endif  // (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
}

x11_return_type x11_appRootWindow(const int screen)
{
#ifdef QF_HAS_X11EXTRAS
    return QX11Info::appRootWindow(screen);
#endif

    if (!qApp)
        return 0;

    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native)
        return 0;

    QScreen *scr = ((screen == -1) ? QGuiApplication::primaryScreen() : x11_findScreenForVirtualDesktop(screen));
    if (!scr)
        return 0;
    return static_cast<xcb_window_t>(reinterpret_cast<quintptr>(native->nativeResourceForScreen("rootwindow", scr)));
}

QScreen *x11_findScreenForVirtualDesktop(const int virtualDesktopNumber)
{
    if (-1 == virtualDesktopNumber)
        return QGuiApplication::primaryScreen();

    QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty())
        return nullptr;

    for (QScreen *&screen : screens)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        const auto qxcbScreen = dynamic_cast<QNativeInterface::Private::QXcbScreen *>(screen->handle());
        if (qxcbScreen && (qxcbScreen->virtualDesktopNumber() == virtualDesktopNumber))
            return screen;
#else
        if (QXcbScreenFunctions::virtualDesktopNumber(screen) == virtualDesktopNumber)
            return screen;
#endif
    }
    return nullptr;
}

int x11_appScreen()
{
#ifdef Q_HAS_X11EXTRAS
    return QX11Info::appScreen();
#endif

    if (!qApp)
        return 0;

    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native)
        return 0;
    return reinterpret_cast<qintptr>(native->nativeResourceForIntegration("x11screen"));
}

xcb_atom_t internAtom(const char *name)
{
    Q_ASSERT(name);
    Q_ASSERT(*name != '\0');
    if (!name || (*name == '\0'))
        return XCB_NONE;

    xcb_connection_t * const connection = x11_connection();
    Q_ASSERT(connection);
    if (!connection)
        return XCB_NONE;

    const xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, false, qstrlen(name), name);
    xcb_intern_atom_reply_t * const reply = xcb_intern_atom_reply(connection, cookie, nullptr);
    if (!reply)
        return XCB_NONE;

    const xcb_atom_t atom = reply->atom;
    std::free(reply);
    return atom;
}

bool isSupportedByWindowManager(const xcb_atom_t atom)
{
    Q_ASSERT(atom != XCB_NONE);
    if (atom == XCB_NONE)
        return false;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    using result_type = QList<xcb_atom_t>;
#else   // (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    using result_type = QVector<xcb_atom_t>;
#endif  // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    static const auto netWmAtoms = []() -> result_type
    {
        xcb_connection_t * const connection = x11_connection();
        Q_ASSERT(connection);
        if (!connection)
            return {};

        const quint32 rootWindow = x11_appRootWindow(x11_appScreen());
        Q_ASSERT(rootWindow);
        if (!rootWindow)
            return {};

        static const xcb_atom_t netSupportedAtom = internAtom(ATOM_NET_SUPPORTED);
        if (netSupportedAtom == XCB_NONE)
            return { };

        result_type result = {};
        int offset = 0;
        int remaining = 0;
        do {
            const xcb_get_property_cookie_t cookie = xcb_get_property(connection, false, rootWindow, netSupportedAtom, XCB_ATOM_ATOM, offset, 1024);
            xcb_get_property_reply_t * const reply = xcb_get_property_reply(connection, cookie, nullptr);
            if (!reply)
                break;

            remaining = 0;
            if ((reply->type == XCB_ATOM_ATOM) && (reply->format == 32))
            {
                const int len = (xcb_get_property_value_length(reply) / sizeof(xcb_atom_t));
                const auto atoms = static_cast<xcb_atom_t *>(xcb_get_property_value(reply));
                const int size = result.size();
                result.resize(size + len);
                memcpy(result.data() + size, atoms, len * sizeof(xcb_atom_t));
                remaining = reply->bytes_after;
                offset += len;
            }
            std::free(reply);
        } while (remaining > 0);
        return result;
    }();
    return netWmAtoms.contains(atom);
}

bool isCustomDecorationSupported()
{
    static const xcb_atom_t atom = internAtom(ATOM_DEEPIN_NO_TITLEBAR);
    return ((atom != XCB_NONE) && isSupportedByWindowManager(atom));
}

void showSystemMenu(const WId windowId, const QPoint &globalPos)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return;

    xcb_connection_t * const connection = x11_connection();
    Q_ASSERT(connection);
    if (!connection)
        return;

    const quint32 rootWindow = x11_appRootWindow(x11_appScreen());
    Q_ASSERT(rootWindow);
    if (!rootWindow)
        return;

    static const xcb_atom_t atom = internAtom(ATOM_GTK_SHOW_WINDOW_MENU);
    if ((atom == XCB_NONE) || !isSupportedByWindowManager(atom))
        return;

    xcb_client_message_event_t xev;
    memset(&xev, 0, sizeof(xev));
    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = atom;
    xev.window = windowId;
    xev.format = 32;
    xev.data.data32[1] = globalPos.x();
    xev.data.data32[2] = globalPos.y();

    xcb_ungrab_pointer(connection, XCB_CURRENT_TIME);
    xcb_send_event(connection, false, rootWindow, _XCB_SEND_EVENT_MASK, reinterpret_cast<const char *>(&xev));
    xcb_flush(connection);
}

QF_END_NAMESPACE
