#include "frameless/utils/qframelesshelper_p.h"

#include "frameless/utils/qframelessmanager_p.h"
#include "frameless/utils/qframelessutils_p.h"
#include "frameless/utils/qsysapiloader_p.h"
#include "frameless/utils/qsystemversion_p.h"

#include <Windows.h>
#include <windowsx.h>
#include <Uxtheme.h>
#include <cmath>
#include <QAbstractNativeEventFilter>
#include <QAbstractEventDispatcher>
#include <QScopeGuard>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QWindow>
#include <QDebug>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

#define QF_MESSAGE_TAG WPARAM(0x97CCEA99)

#ifndef WM_UAHDESTROYWINDOW
#   define WM_UAHDESTROYWINDOW (0x0090)
#endif

#ifndef WM_UNREGISTER_WINDOW_SERVICES
#   define WM_UNREGISTER_WINDOW_SERVICES (0x0272)
#endif

#ifndef QF_RECT_WIDTH
#  define QF_RECT_WIDTH(rect) ((rect).right - (rect).left)
#endif

#ifndef QF_RECT_HEIGHT
#  define QF_RECT_HEIGHT(rect) ((rect).bottom - (rect).top)
#endif

constexpr const char kDontToggleMaximizeVar[] = "QF_TOGGLE_MAXIMIZE";
constexpr const char kDontOverrideCursorVar[] = "QF_DONT_OVERRIDE_CURSOR";

namespace
{
/**
     * @brief The QFramelessHelperWin class 事件过滤器
     */
class QFramelessHelperWin : public QAbstractNativeEventFilter
{
public:
    ~QFramelessHelperWin() override = default;
    bool nativeEventFilter(const QByteArray& eventType, void* message, QT_NATIVE_EVENT_RESULT_TYPE* result) override;
};

/**
 * @brief The QFramelessEventFilter struct windows系统无边框内部助手
 */
struct QFramelessHelperWinInternal
{
    std::unique_ptr<QFramelessHelperWin> eventFilter{ };
};
Q_GLOBAL_STATIC(QFramelessHelperWinInternal, g_internalData)

/**
 * @brief The WindowPart enum 窗口组成部分
 */
enum class WindowPart : quint8
{
    Outside,
    ClientArea,
    ChromeButton,
    ResizeBorder,
    FixedBorder,
    TitleBar
};

/**
 * @brief The QFramelessDataWin struct windows系统无边框窗口信息
 */
struct QFramelessDataWin : public QFramelessData
{
    WindowPart lastHitTestResult{ WindowPart::Outside };
    QF::Dpi dpi{ };
    HMONITOR monitor{ };
    bool isMouseLeaveBlocked { false };

    QFramelessDataWin() = default;
    ~QFramelessDataWin() override = default;
};
using QFramelessDataWinPtr = std::shared_ptr<QFramelessDataWin>;

/**
 * @brief The QUtilsWinExtraData struct windows系统回调函数
 */
struct QUtilsWinExtraData : public QFramelessExtraData
{
    WNDPROC qtWindowProc = nullptr;
    bool windowProcHooked = false;
    bool mica = false;

    QUtilsWinExtraData() = default;
    ~QUtilsWinExtraData() override = default;

    static QFramelessExtraDataPtr create() { return std::make_shared<QUtilsWinExtraData>(); }
};
using QUtilsWinExtraDataPtr = std::shared_ptr<QUtilsWinExtraData>;

}

QFramelessData::PtrType QFramelessData::create()
{
    return std::make_shared<QFramelessDataWin>();
}

static inline QFramelessDataWinPtr tryGetWinData(const QObject* window)
{
    Q_ASSERT(window);
    if (nullptr == window)
        return nullptr;

    const QFramelessDataPtr data = QFramelessManager::data(window);
    if (!data)
        return nullptr;
    return std::dynamic_pointer_cast<QFramelessDataWin>(data);
}

static QUtilsWinExtraDataPtr tryGetExtraData(const QFramelessDataPtr& data, const bool create)
{
    Q_ASSERT(data);
    if (!data)
        return nullptr;

    auto it = data->extraData.find(QF::ExtraDataType::WindowsUtilities);
    if (data->extraData.end() == it)
    {
        if (create)
            it = data->extraData.insert(QF::ExtraDataType::WindowsUtilities, QUtilsWinExtraData::create());
        else
            return nullptr;
    }
    return std::dynamic_pointer_cast<QUtilsWinExtraData>(it.value());
}

static inline WindowPart getHittedWindowPart(const int hitTestResult)
{
    switch (hitTestResult)
    {
    case HTCLIENT:
        return WindowPart::ClientArea;
    case HTCAPTION:
        return WindowPart::TitleBar;
    case HTSYSMENU:
    case HTHELP:
    case HTREDUCE:
    case HTZOOM:
    case HTCLOSE:
        return WindowPart::ChromeButton;
    case HTLEFT:
    case HTRIGHT:
    case HTTOP:
    case HTTOPLEFT:
    case HTTOPRIGHT:
    case HTBOTTOM:
    case HTBOTTOMLEFT:
    case HTBOTTOMRIGHT:
        return WindowPart::ResizeBorder;
    case HTBORDER:
        return WindowPart::FixedBorder;
    default:
        break;
    }
    return WindowPart::Outside;
}

static void emulateClientAreaMessage(HWND hd, UINT uMsg, WPARAM wParam, LPARAM lParam, int* overrideMessage = nullptr)
{
    const int myMsg = nullptr == overrideMessage ? uMsg : *overrideMessage;
    const auto wparam = [myMsg, wParam]() -> WPARAM {
        if (WM_NCMOUSELEAVE == myMsg)
            return QF_MESSAGE_TAG;

        const quint64 keyState = QF::getKeyState();
        if ((myMsg >= WM_NCXBUTTONDOWN) && (myMsg <= WM_NCXBUTTONDBLCLK))
        {
            const auto xButtonMask = GET_XBUTTON_WPARAM(wParam);
            return MAKEWPARAM(keyState, xButtonMask);
        }
        return keyState;
    }();

    const auto lparam = [myMsg, lParam, hd]() -> LPARAM {
        if (WM_NCMOUSELEAVE == myMsg)
            return 0;

        const auto screenPos = POINT{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        POINT clientPos = screenPos;
        if (::ScreenToClient(hd, &clientPos) == FALSE)
            return 0;
        return MAKELPARAM(clientPos.x, clientPos.y);
    }();

#if 0
#  define SEND_MESSAGE ::SendMessageW
#else
#  define SEND_MESSAGE ::PostMessageW
#endif

    switch (myMsg)
    {
    case WM_NCHITTEST: // Treat hit test messages as mouse move events.
    case WM_NCMOUSEMOVE:
        SEND_MESSAGE(hd, WM_MOUSEMOVE, wparam, lparam);
        break;
    case WM_NCLBUTTONDOWN:
        SEND_MESSAGE(hd, WM_LBUTTONDOWN, wparam, lparam);
        break;
    case WM_NCLBUTTONUP:
        SEND_MESSAGE(hd, WM_LBUTTONUP, wparam, lparam);
        break;
    case WM_NCLBUTTONDBLCLK:
        SEND_MESSAGE(hd, WM_LBUTTONDBLCLK, wparam, lparam);
        break;
    case WM_NCRBUTTONDOWN:
        SEND_MESSAGE(hd, WM_RBUTTONDOWN, wparam, lparam);
        break;
    case WM_NCRBUTTONUP:
        SEND_MESSAGE(hd, WM_RBUTTONUP, wparam, lparam);
        break;
    case WM_NCRBUTTONDBLCLK:
        SEND_MESSAGE(hd, WM_RBUTTONDBLCLK, wparam, lparam);
        break;
    case WM_NCMBUTTONDOWN:
        SEND_MESSAGE(hd, WM_MBUTTONDOWN, wparam, lparam);
        break;
    case WM_NCMBUTTONUP:
        SEND_MESSAGE(hd, WM_MBUTTONUP, wparam, lparam);
        break;
    case WM_NCMBUTTONDBLCLK:
        SEND_MESSAGE(hd, WM_MBUTTONDBLCLK, wparam, lparam);
        break;
    case WM_NCXBUTTONDOWN:
        SEND_MESSAGE(hd, WM_XBUTTONDOWN, wparam, lparam);
        break;
    case WM_NCXBUTTONUP:
        SEND_MESSAGE(hd, WM_XBUTTONUP, wparam, lparam);
        break;
    case WM_NCXBUTTONDBLCLK:
        SEND_MESSAGE(hd, WM_XBUTTONDBLCLK, wparam, lparam);
        break;
#if 0 // ### TODO: How to handle touch events?
    case WM_NCPOINTERUPDATE:
    case WM_NCPOINTERDOWN:
    case WM_NCPOINTERUP:
        break;
#endif
    case WM_NCMOUSEHOVER:
        SEND_MESSAGE(hd, WM_MOUSEHOVER, wparam, lparam);
        break;
    case WM_NCMOUSELEAVE:
        SEND_MESSAGE(hd, WM_MOUSELEAVE, wparam, lparam);
        break;
    default:
        Q_UNREACHABLE();
    }
}

static inline bool requestForMouseLeaveMessage(const HWND hWnd, const bool nonClient)
{
    Q_ASSERT(hWnd);
    if (!hWnd)
        return false;

    TRACKMOUSEEVENT tme;
    SecureZeroMemory(&tme, sizeof(tme));
    tme.cbSize = sizeof(tme);
    tme.dwFlags = TME_LEAVE;
    if (nonClient)
        tme.dwFlags |= TME_NONCLIENT;

    tme.hwndTrack = hWnd;
    tme.dwHoverTime = HOVER_DEFAULT;
    if (FALSE == ::TrackMouseEvent(&tme))
        return false;
    return true;
}

static inline QByteArray qtNativeEventType()
{
    static const auto result = QByteArrayLiteral("windows_generic_MSG");
    return result;
}

static inline constexpr bool isNonClientMessage(const UINT message)
{
    if (((message >= WM_NCCREATE) && (message <= WM_NCACTIVATE))
            || ((message >= WM_NCMOUSEMOVE) && (message <= WM_NCMBUTTONDBLCLK))
            || ((message >= WM_NCXBUTTONDOWN) && (message <= WM_NCXBUTTONDBLCLK))
        #if (WINVER >= _WIN32_WINNT_WIN8)
            || ((message >= WM_NCPOINTERUPDATE) && (message <= WM_NCPOINTERUP))
        #endif
            || ((message == WM_NCMOUSEHOVER) || (message == WM_NCMOUSELEAVE))) {
        return true;
    }
    return false;
}

static bool showSystemMenu(const WId windowId, const QPoint& pos, const bool selectFirstEntry)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return false;

    const QObject* window = QFramelessManager::window(windowId);
    if (!window)
        return false;

    const QFramelessDataPtr data = QFramelessManager::data(window);
    if (!data || !data->isFrameless || !data->callbacks)
        return false;

    const auto hWnd = reinterpret_cast<HWND>(windowId);
    const HMENU hMenu = ::GetSystemMenu(hWnd, FALSE);
    if (!hMenu)
        return true;

    const bool disableClose = data->callbacks->getProperty(kSysMenuDisableCloseVar, false).toBool();
    const bool disableRestore = data->callbacks->getProperty(kSysMenuDisableRestoreVar, false).toBool();
    const bool disableMinimize = data->callbacks->getProperty(kSysMenuDisableMinimizeVar, false).toBool();
    const bool disableMaximize = data->callbacks->getProperty(kSysMenuDisableMaximizeVar, false).toBool();
    const bool disableSize = data->callbacks->getProperty(kSysMenuDisableSizeVar, false).toBool();
    const bool disableMove = data->callbacks->getProperty(kSysMenuDisableMoveVar, false).toBool();
    const bool removeClose = data->callbacks->getProperty(kSysMenuRemoveCloseVar, false).toBool();
    const bool removeSeparator = data->callbacks->getProperty(kSysMenuRemoveSeparatorVar, false).toBool();
    const bool removeRestore = data->callbacks->getProperty(kSysMenuRemoveRestoreVar, false).toBool();
    const bool removeMinimize = data->callbacks->getProperty(kSysMenuRemoveMinimizeVar, false).toBool();
    const bool removeMaximize = data->callbacks->getProperty(kSysMenuRemoveMaximizeVar, false).toBool();
    const bool removeSize = data->callbacks->getProperty(kSysMenuRemoveSizeVar, false).toBool();
    const bool removeMove = data->callbacks->getProperty(kSysMenuRemoveMoveVar, false).toBool();
    const bool maxOrFull = (IsMaximized(hWnd) || QF::isFullScreen(windowId));
    const bool fixedSize = data->callbacks->isWindowFixedSize();
    if (removeClose)
        ::DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
    else
        ::EnableMenuItem(hMenu, SC_CLOSE, (MF_BYCOMMAND | (disableClose ? MFS_DISABLED : MFS_ENABLED)));

    if (removeSeparator)
        ::DeleteMenu(hMenu, 0, MFT_SEPARATOR);

    if (removeMaximize)
        ::DeleteMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
    else
        ::EnableMenuItem(hMenu, SC_MAXIMIZE, (MF_BYCOMMAND | ((!maxOrFull && !fixedSize && !disableMaximize) ? MFS_ENABLED : MFS_DISABLED)));

    if (removeRestore)
        ::DeleteMenu(hMenu, SC_RESTORE, MF_BYCOMMAND);
    else
    {
        ::EnableMenuItem(hMenu, SC_RESTORE, (MF_BYCOMMAND | ((maxOrFull && !fixedSize && !disableRestore) ? MFS_ENABLED : MFS_DISABLED)));
        ::HiliteMenuItem(hWnd, hMenu, SC_RESTORE, (MF_BYCOMMAND | (selectFirstEntry ? MFS_HILITE : MFS_UNHILITE)));
    }

    if (removeMinimize)
        ::DeleteMenu(hMenu, SC_MINIMIZE, MF_BYCOMMAND);
    else
        ::EnableMenuItem(hMenu, SC_MINIMIZE, (MF_BYCOMMAND | (disableMinimize ? MFS_DISABLED : MFS_ENABLED)));

    if (removeSize)
        ::DeleteMenu(hMenu, SC_SIZE, MF_BYCOMMAND);
    else
        ::EnableMenuItem(hMenu, SC_SIZE, (MF_BYCOMMAND | ((!maxOrFull && !fixedSize && !(disableSize || disableMinimize || disableMaximize)) ? MFS_ENABLED : MFS_DISABLED)));

    if (removeMove)
        ::DeleteMenu(hMenu, SC_MOVE, MF_BYCOMMAND);
    else
        ::EnableMenuItem(hMenu, SC_MOVE, (MF_BYCOMMAND | ((disableMove || maxOrFull) ? MFS_DISABLED : MFS_ENABLED)));

    bool isOk = false;
    UINT defaultItemId;
    if (QSystemVersion::isWin11OrGreater())
    {
        if (maxOrFull)
        {
            if (!removeRestore)
            {
                isOk = true;
                defaultItemId = SC_RESTORE;
            }
        }
        else
        {
            if (!removeMaximize)
            {
                isOk = true;
                defaultItemId = SC_MAXIMIZE;
            }
        }
    }

    if (!isOk || removeClose)
    {
        isOk = true;
        defaultItemId = SC_CLOSE;
    }

    if (FALSE == ::SetMenuDefaultItem(hMenu, isOk ? defaultItemId : UINT_MAX, FALSE))
        qWarning().noquote() << __FUNCTION__ << "Failed to set menu defualt item!";

    if (FALSE == ::DrawMenuBar(hWnd))
        qWarning().noquote() << __FUNCTION__ << "Failed to draw menu bar";

    const int result = ::TrackPopupMenu(hMenu, (TPM_RETURNCMD | (QGuiApplication::isRightToLeft() ? TPM_RIGHTALIGN : TPM_LEFTALIGN)), pos.x(), pos.y(), 0, hWnd, nullptr);
    if (!removeRestore)
        ::HiliteMenuItem(hWnd, hMenu, SC_RESTORE, (MF_BYCOMMAND | MFS_UNHILITE));

    if (FALSE == result)
        return true;

    if (FALSE == ::PostMessageW(hWnd, WM_SYSCOMMAND, result, 0))
    {
        qWarning().noquote() << __FUNCTION__ << "Failed to send the command that the user choses to the corresponding window.";
        return false;
    }

    return true;
}

static LRESULT CALLBACK FramelessHelperHookWindowProc(const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
    Q_ASSERT(hWnd);
    if (!hWnd)
        return FALSE;

    const auto defaultWindowProcessing = [hWnd, uMsg, wParam, lParam]() -> LRESULT { return ::DefWindowProcW(hWnd, uMsg, wParam, lParam); };
    const auto windowId = reinterpret_cast<WId>(hWnd);
    const QObject* window = QFramelessManager::window(windowId);
    if (!window)
        return defaultWindowProcessing();

    const QFramelessDataPtr data = QFramelessManager::data(window);
    if (!data || !data->isFrameless || !data->callbacks)
        return defaultWindowProcessing();

    const QUtilsWinExtraDataPtr extraData = tryGetExtraData(data, false);
    Q_ASSERT(extraData);
    if (!extraData)
        return defaultWindowProcessing();

    const QWindow* qWindow = data->callbacks->getWindowHandle();
    Q_ASSERT(qWindow);
    if (!qWindow)
        return defaultWindowProcessing();

    if (WM_NCCALCSIZE != uMsg)
    {
        MSG message;
        SecureZeroMemory(&message, sizeof(message));
        message.hwnd = hWnd;
        message.message = uMsg;
        message.wParam = wParam;
        message.lParam = lParam;
        message.time = ::GetMessageTime();
#if 1
        const DWORD dwScreenPos = ::GetMessagePos();
        message.pt.x = GET_X_LPARAM(dwScreenPos);
        message.pt.y = GET_Y_LPARAM(dwScreenPos);
#else
        if (::GetCursorPos(&message.pt) == FALSE)
            message.pt = {};
#endif

        if (!isNonClientMessage(uMsg))
        {
            if (FALSE == ::ScreenToClient(hWnd, &message.pt))
                message.pt = {};
        }

        QT_NATIVE_EVENT_RESULT_TYPE filterResult = 0;
        const auto dispatcher = QAbstractEventDispatcher::instance();
        if (dispatcher && dispatcher->filterNativeEvent(qtNativeEventType(), &message, &filterResult))
            return LRESULT(filterResult);
    }

    const auto getNativePosFromMouse = [lParam]() -> QPoint {
            return { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
};

    const auto getNativeGlobalPosFromKeyboard = [hWnd, windowId]() -> QPoint {
        RECT windowPos = {};
        if (FALSE == ::GetWindowRect(hWnd, &windowPos))
            return {};

        const bool maxOrFull = (IsMaximized(hWnd) || QF::isFullScreen(windowId));
        const int frameSizeX = QF::getResizeBorderThickness(windowId, true, true);
        const bool frameBorderVisible = QF::isWindowFrameBorderVisible();
        const int horizontalOffset = ((maxOrFull || !frameBorderVisible) ? 0 : frameSizeX);
        const auto verticalOffset = [windowId, frameBorderVisible, maxOrFull]() -> int {
            const int titleBarHeight = QF::getTitleBarHeight(windowId, true);
            if (!frameBorderVisible)
                return titleBarHeight;

            const int frameSizeY = QF::getResizeBorderThickness(windowId, false, true);
            if (QSystemVersion::isWin11OrGreater())
            {
                if (maxOrFull)
                    return (titleBarHeight + frameSizeY);
                return titleBarHeight;
            }

            if (maxOrFull)
                return titleBarHeight;
            return (titleBarHeight - frameSizeY);
        }();
        return { windowPos.left + horizontalOffset, windowPos.top + verticalOffset };
    };

    bool shouldShowSystemMenu = false;
    bool broughtByKeyboard = false;
    QPoint nativeGlobalPos = {};
    switch (uMsg)
    {
    case WM_RBUTTONUP:
    {
        const QPoint nativeLocalPos = getNativePosFromMouse();
        const QPoint qtScenePos = QF::fromNativeLocalPosition(qWindow, nativeLocalPos);
        if (data->callbacks->isInsideTitleBarDraggableArea(qtScenePos))
        {
            POINT pos = { nativeLocalPos.x(), nativeLocalPos.y() };
            if (FALSE == ::ClientToScreen(hWnd, &pos))
                break;
            shouldShowSystemMenu = true;
            nativeGlobalPos = { pos.x, pos.y };
        }
        break;
    }
    case WM_NCRBUTTONUP:
        if (wParam == HTCAPTION)
        {
            shouldShowSystemMenu = true;
            nativeGlobalPos = getNativePosFromMouse();
        }
        break;
    case WM_SYSCOMMAND:
    {
        const WPARAM filteredWParam = (wParam & 0xFFF0);
        if ((filteredWParam == SC_KEYMENU) && (lParam == VK_SPACE))
        {
            shouldShowSystemMenu = true;
            broughtByKeyboard = true;
            nativeGlobalPos = getNativeGlobalPosFromKeyboard();
        }
        break;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        const bool altPressed = ((wParam == VK_MENU) || (::GetKeyState(VK_MENU) < 0));
        const bool spacePressed = ((wParam == VK_SPACE) || (::GetKeyState(VK_SPACE) < 0));
        if (altPressed && spacePressed) {
            shouldShowSystemMenu = true;
            broughtByKeyboard = true;
            nativeGlobalPos = getNativeGlobalPosFromKeyboard();
        }
        break;
    }
    default:
        break;
    }

    if (shouldShowSystemMenu)
    {
        showSystemMenu(windowId, nativeGlobalPos, broughtByKeyboard);
        return FALSE;
    }

    Q_ASSERT(extraData->qtWindowProc);
    if (extraData->qtWindowProc)
        return ::CallWindowProcW(extraData->qtWindowProc, hWnd, uMsg, wParam, lParam);
    return defaultWindowProcessing();
}

static bool installWindowProcHook(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return false;

    const QObject* window = QFramelessManager::window(windowId);
    Q_ASSERT(window);
    if (!window)
        return false;

    const QFramelessDataPtr data = QFramelessManager::data(window);
    Q_ASSERT(data);
    if (!data || data->isFrameless)
        return false;

    const QUtilsWinExtraDataPtr extraData = tryGetExtraData(data, true);
    Q_ASSERT(extraData);
    if (!extraData)
        return false;

    const auto hwnd = reinterpret_cast<HWND>(windowId);
    if (!extraData->qtWindowProc)
    {
        ::SetLastError(ERROR_SUCCESS);
        const auto qtWindowProc = reinterpret_cast<WNDPROC>(::GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
        Q_ASSERT(qtWindowProc);
        if (!qtWindowProc)
            return false;
        extraData->qtWindowProc = qtWindowProc;
    }

    if (!extraData->windowProcHooked)
    {
        ::SetLastError(ERROR_SUCCESS);
        if (::SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(FramelessHelperHookWindowProc)) == 0)
            return false;
        extraData->windowProcHooked = true;
    }

    return true;
}

static bool uninstallWindowProcHook(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return false;

    const QObject* window = QFramelessManager::window(windowId);
    Q_ASSERT(window);
    if (!window)
        return false;

    const QFramelessDataPtr data = QFramelessManager::data(window);
    Q_ASSERT(data);
    if (!data || !data->isFrameless)
        return false;

    const QUtilsWinExtraDataPtr extraData = tryGetExtraData(data, false);
    Q_ASSERT(extraData);
    if (!extraData)
        return false;

    const auto hwnd = reinterpret_cast<HWND>(windowId);
    ::SetLastError(ERROR_SUCCESS);
    if (::SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(extraData->qtWindowProc)) == 0)
        return false;

    extraData->qtWindowProc = nullptr;
    extraData->windowProcHooked = false;
    return true;
}

static bool updateWindowFrameMargins(const WId windowId, bool reset)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return 0;

    if (!QSysApiLoader::isAvailable("dwmapi", "DwmExtendFrameIntoClientArea"))
        return false;

    if (!QF::isDwmCompositionEnabled())
        return false;

    const QObject* window = QFramelessManager::window(windowId);
    if (!window)
        return false;

    const QFramelessDataPtr data = QFramelessManager::data(window);
    if (!data)
        return false;

    const QUtilsWinExtraDataPtr extraData = tryGetExtraData(data, false);
    Q_ASSERT(extraData);
    if (!extraData)
        return false;

    const auto margins = [&extraData, reset]() -> MARGINS {
        if (extraData->mica)
            return { -1, -1, -1, -1 };

        if (reset || QF::isWindowFrameBorderVisible())
            return { 0, 0, 0, 0 };

        return { 1, 1, 1, 1 };
    }();

    const auto hwnd = reinterpret_cast<HWND>(windowId);
    typedef HRESULT(WINAPI* DwmExtendFrameIntoClientAreaPtr)(HWND, const MARGINS*);
    const HRESULT hr = QSysApiLoader::get<DwmExtendFrameIntoClientAreaPtr>("dwmapi", "DwmExtendFrameIntoClientArea")(hwnd, &margins);
    if (FAILED(hr))
        return false;
    return QF::triggerFrameChange(windowId);
}

void QFramelessHelperPrivate::initQtGlobalAttributes()
{
    /**
     * @brief QF::tryToEnableHighestDpiAwarenessLevel
     *      This is equivalent to set the "dpiAware" and "dpiAwareness" field in
     * your manifest file. It works through out Windows Vista to Windows 11.
     * It's highly recommended to enable the highest DPI awareness mode
     * (currently it's PerMonitor Version 2, or PMv2 for short) for any GUI
     * applications, to allow your user interface scale to an appropriate
     * size and still stay sharp, though you will have to do the calculation
     * and resize by yourself.
     */
    QF::tryToEnableHighestDpiAwarenessLevel();

    /**
     * @brief QF::fixupDialogsDpiScaling
     *      This function need to be called before any dialogs are created, so
     * to be safe we call it here.
     * Without this hack, our native dialogs won't be able to respond to
     * DPI change messages correctly, especially the non-client area.
     */
    QF::fixupDialogsDpiScaling();
}

void QFramelessHelperPrivate::showSystemMenu(const QPoint &point)
{
    if (!window)
        return;

    const WId windowId = window->winId();
    const QPoint nativePos = QF::toNativeGlobalPosition(window->windowHandle(), point);
    ::showSystemMenu(windowId, nativePos, false);
}

void QFramelessHelperPrivate::setupFramelessWindow(const QObject* window)
{
    Q_ASSERT(window);
    if (nullptr == window)
        return;

    const QFramelessDataWinPtr data = tryGetWinData(window);
    if (!data || data->isFrameless || !data->callbacks)
        return;

    installWindowProcHook(data->windowId);

    QWindow* qWindow = data->callbacks->getWindowHandle();
    if (!qWindow)
        return;

    data->isFrameless = true;
    data->dpi = QF::Dpi{ QF::getWindowDpi(data->windowId, true), QF::getWindowDpi(data->windowId, false) };
    data->monitor = ::MonitorFromWindow(reinterpret_cast<HWND>(data->windowId), MONITOR_DEFAULTTONEAREST);
    Q_ASSERT(data->monitor);

#if ((QT_VERSION != QT_VERSION_CHECK(6, 5, 3)) && (QT_VERSION != QT_VERSION_CHECK(6, 6, 0)))
    QF::updateInternalWindowFrameMargins(qWindow, true);
#else
    data->callbacks->setWindowFlags(data->callbacks->getWindowFlags() | Qt::FramelessWindowHint);
#endif

    QF::hideOriginalTitleBarElements(data->windowId, true);
    QF::fixupChildWindowsDpiMessage(data->windowId);
    updateWindowFrameMargins(data->windowId, false);

    if (!g_internalData()->eventFilter)
    {
        g_internalData()->eventFilter = std::make_unique<QFramelessHelperWin>();
        qApp->installNativeEventFilter(g_internalData()->eventFilter.get());
    }
}

void QFramelessHelperPrivate::teardownFramelessWindow(const QObject* window)
{
    Q_ASSERT(window);
    if (nullptr == window)
        return;

    const QFramelessDataWinPtr data = tryGetWinData(window);
    if (!data || !data->isFrameless || !data->callbacks)
        return;

    uninstallWindowProcHook(data->windowId);
    data->isFrameless = false;
}

bool QFramelessHelperWin::nativeEventFilter(const QByteArray& eventType, void* message, QT_NATIVE_EVENT_RESULT_TYPE* result)
{
    if (eventType != qtNativeEventType() || !message || !result)
        return false;

#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    const auto msg = *static_cast<MSG**>(message);
#else
    const auto msg = static_cast<const MSG*>(message);
#endif

    const HWND hWnd = msg->hwnd;
    if (!hWnd)
        return false;

    const UINT uMsg = msg->message;
    switch (uMsg)
    {
    case WM_CLOSE:
    case WM_DESTROY:
    case WM_NCDESTROY:
    case WM_UAHDESTROYWINDOW:
    case WM_UNREGISTER_WINDOW_SERVICES:
        return false;
    default:
        break;
    }

    const auto winId = reinterpret_cast<WId>(hWnd);
    if (!QF::isValidWindow(winId, false, true))
        return false;

    const QObject* window = QFramelessManager::window(winId);
    if (!window)
        return false;

    const QFramelessDataWinPtr data = tryGetWinData(window);
    if (!data || !data->isFrameless || !data->callbacks)
        return false;

    const WPARAM wParam = msg->wParam;
    const LPARAM lParam = msg->lParam;
    QWindow* qWindow = data->callbacks->getWindowHandle();
    const bool isFrameBorderVisible = QF::isWindowFrameBorderVisible();
    switch (uMsg)
    {
    case WM_NCCALCSIZE:
    {
        const auto clientRect = ((wParam == FALSE) ? reinterpret_cast<LPRECT>(lParam) : &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam))->rgrc[0]);
        RECT origRect = *clientRect;
        const LRESULT hitTestResult = ::DefWindowProcW(hWnd, WM_NCCALCSIZE, wParam, lParam);
        if ((hitTestResult != HTERROR) && (hitTestResult != HTNOWHERE))
        {
            *result = hitTestResult;
            return true;
        }

        if (!isFrameBorderVisible)
        {
            *clientRect = origRect;
            clientRect->top = origRect.top - 1;
        }
        else
            clientRect->top = origRect.top;

        bool isMaxi = ::IsZoomed(hWnd) != FALSE;
        bool isFull = QF::isFullScreen(winId);
        if (isMaxi && !isFull)
        {
            const int frameSizeX = QF::getResizeBorderThickness(winId, true, true);
            const int frameSizeY = QF::getResizeBorderThickness(winId, false, true);
            clientRect->top += frameSizeY;
            clientRect->bottom -= frameSizeY;
            clientRect->right -= frameSizeX;
            clientRect->left += frameSizeX;
        }

        if (isMaxi || isFull)
        {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            const UINT taskbarState = ::SHAppBarMessage(ABM_GETSTATE, &abd);
            if (taskbarState & ABS_AUTOHIDE)
            {
                bool top = false, bottom = false, left = false, right = false;
                if (QSystemVersion::isWin8Point1OrGreater())
                {
                    MONITORINFOEXW mi;
                    if (!QF::getMonitorForWindow(hWnd, mi))
                        break;

                    const RECT monitorRect = mi.rcMonitor;
                    const auto hasAutohideTaskbar = [monitorRect](const UINT edge) -> bool {
                        APPBARDATA abd2;
                        SecureZeroMemory(&abd2, sizeof(abd2));
                        abd2.cbSize = sizeof(abd2);
                        abd2.uEdge = edge;
                        abd2.rc = monitorRect;
                        const auto hTaskbar = reinterpret_cast<HWND>(::SHAppBarMessage(ABM_GETAUTOHIDEBAREX, &abd2));
                        return (hTaskbar != nullptr);
                    };
                    top = hasAutohideTaskbar(ABE_TOP);
                    bottom = hasAutohideTaskbar(ABE_BOTTOM);
                    left = hasAutohideTaskbar(ABE_LEFT);
                    right = hasAutohideTaskbar(ABE_RIGHT);
                }
                else
                {
                    int edge = -1;
                    APPBARDATA abd2;
                    SecureZeroMemory(&abd2, sizeof(abd2));
                    abd2.cbSize = sizeof(abd2);
                    abd2.hWnd = ::FindWindowW(L"Shell_TrayWnd", nullptr);
                    if (abd2.hWnd)
                    {
                        const HMONITOR windowMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
                        if (!windowMonitor)
                            break;

                        const HMONITOR taskbarMonitor = ::MonitorFromWindow(abd2.hWnd, MONITOR_DEFAULTTOPRIMARY);
                        if (!taskbarMonitor)
                            break;

                        if (taskbarMonitor == windowMonitor)
                        {
                            ::SHAppBarMessage(ABM_GETTASKBARPOS, &abd2);
                            edge = abd2.uEdge;
                        }
                    }
                    top = (edge == ABE_TOP);
                    bottom = (edge == ABE_BOTTOM);
                    left = (edge == ABE_LEFT);
                    right = (edge == ABE_RIGHT);
                }

                if (top)
                    clientRect->top += 2;
                else if (bottom)
                    clientRect->bottom -= 2;
                else if (left)
                    clientRect->left += 2;
                else if (right)
                    clientRect->right -= 2;
            }
        }

        QF::updateAllDirectXSurfaces();
        QF::syncWmPaintWithDwm();

        *result = wParam == FALSE ? FALSE : WVR_REDRAW;
        return true;
    }
    case WM_NCHITTEST:
    {
        const auto hitTestRecorder = qScopeGuard([&data, &result]() {
            data->lastHitTestResult = getHittedWindowPart(*result);
        });

        const auto nativeGlobalPos = POINT{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        POINT nativeLocalPos = nativeGlobalPos;
        if (FALSE == ::ScreenToClient(hWnd, &nativeLocalPos))
            break;

        auto clientRect = RECT{ 0, 0, 0, 0 };
        if (FALSE == ::GetClientRect(hWnd, &clientRect))
            break;

        const auto clientWidth = QF_RECT_WIDTH(clientRect);
        const auto clientHeight = QF_RECT_HEIGHT(clientRect);

        const QPoint qtScenePos = QF::fromNativeLocalPosition(qWindow, QPoint(nativeLocalPos.x, nativeLocalPos.y));

        const bool isFixedSize = data->callbacks->isWindowFixedSize();
        const bool isTitleBar = data->callbacks->isInsideTitleBarDraggableArea(qtScenePos);
        const bool dontOverrideCursor = data->callbacks->getProperty(kDontOverrideCursorVar, false).toBool();
        const bool dontToggleMaximize = data->callbacks->getProperty(kDontToggleMaximizeVar, false).toBool();
        if (dontToggleMaximize)
        {
            static bool warnedOnce = false;
            if (!warnedOnce)
                warnedOnce = true;
        }

        QSystemButton::Type sysBtnType = QSystemButton::Invalid;
        if (!isFixedSize && data->callbacks->isInsideSystemButtons(qtScenePos, &sysBtnType))
        {
            *result = HTNOWHERE;
            if (QF::isWindowNoState(winId))
            {
                static constexpr const int kBorderSize = 2;
                const bool isTop = (nativeLocalPos.y <= kBorderSize);
                const bool isRight = (nativeLocalPos.x >= (clientWidth - kBorderSize));
                if (isTop || isRight)
                {
                    if (!dontOverrideCursor)
                    {
                        if (isTop && isRight)
                            *result = HTTOPRIGHT;
                        else if (isTop)
                            *result = HTTOP;
                        else
                            *result = HTRIGHT;
                    }
                    else
                        *result = (isTitleBar ? HTCAPTION : HTCLIENT);
                }
            }

            if (HTNOWHERE == *result)
            {
                switch (sysBtnType)
                {
                case QSystemButton::WindowIcon:
                    *result = HTSYSMENU;
                    break;
                case QSystemButton::Help:
                    *result = HTHELP;
                    break;
                case QSystemButton::Minimize:
                    *result = HTREDUCE;
                    break;
                case QSystemButton::Maximize:
                case QSystemButton::Restore:
                    *result = HTZOOM;
                    break;
                case QSystemButton::Close:
                    *result = HTCLOSE;
                    break;
                default:
                    *result = HTCLIENT;
                    break;
                }
            }
            return true;
        }

        bool isMaxi = ::IsZoomed(hWnd) != FALSE;
        bool isFull = QF::isFullScreen(winId);
        const LRESULT originalHitTestResult = ::DefWindowProcW(hWnd, WM_NCHITTEST, 0, lParam);
        if (HTCLIENT != originalHitTestResult)
        {
            *result = ((isFixedSize || dontOverrideCursor) ? HTBORDER : originalHitTestResult);
            return true;
        }

        if (isFull)
        {
            *result = HTCLIENT;
            return true;
        }

        if (isMaxi)
        {
            *result = (isTitleBar ? HTCAPTION : HTCLIENT);
            return true;
        }

        if (!isFixedSize)
        {
            const int frameSizeX = QF::getResizeBorderThickness(winId, true, true);
            const int frameSizeY = QF::getResizeBorderThickness(winId, false, true);
            const bool isTop = (nativeLocalPos.y < frameSizeY);
            const bool isBottom = (nativeLocalPos.y >= (clientHeight - frameSizeY));
            const auto scaleFactor = ((isTop || isBottom) ? qreal(2) : qreal(1));
            const int scaledFrameSizeX = std::round(qreal(frameSizeX) * scaleFactor);
            const bool isLeft = (nativeLocalPos.x < scaledFrameSizeX);
            const bool isRight = (nativeLocalPos.x >= (clientWidth - scaledFrameSizeX));
            if (dontOverrideCursor && (isTop || isBottom || isLeft || isRight))
            {
                *result = (isTitleBar ? HTCAPTION : HTCLIENT);
                return true;
            }

            if (isTop)
            {
                if (isLeft)
                {
                    *result = HTTOPLEFT;
                    return true;
                }

                if (isRight)
                {
                    *result = HTTOPRIGHT;
                    return true;
                }
                *result = HTTOP;
                return true;
            }

            if (isBottom)
            {
                if (isLeft)
                {
                    *result = HTBOTTOMLEFT;
                    return true;
                }

                if (isRight)
                {
                    *result = HTBOTTOMRIGHT;
                    return true;
                }
                *result = HTBOTTOM;
                return true;
            }

            if (isLeft)
            {
                *result = HTLEFT;
                return true;
            }

            if (isRight)
            {
                *result = HTRIGHT;
                return true;
            }
        }

        if (isTitleBar)
        {
            *result = HTCAPTION;
            return true;
        }

        *result = HTCLIENT;
        return true;
    }
    case WM_MOUSELEAVE:
    {
        if (QF_MESSAGE_TAG != wParam)
        {
            const QPoint qtScenePos = QF::fromNativeLocalPosition(qWindow, QPoint{ msg->pt.x, msg->pt.y });
            QSystemButton::Type dummy = QSystemButton::Invalid;
            if (data->callbacks->isInsideSystemButtons(qtScenePos, &dummy))
            {
                data->isMouseLeaveBlocked = true;
                *result = FALSE;
                return true;
            }
        }
        data->isMouseLeaveBlocked = false;
        break;
    }
    case WM_MOUSEMOVE:
    {
        if (WindowPart::ChromeButton != data->lastHitTestResult && data->isMouseLeaveBlocked)
        {
            data->isMouseLeaveBlocked = false;
            requestForMouseLeaveMessage(hWnd, false);
        }
        break;
    }
    case WM_SYSCOMMAND:
    {
        const bool isFixedSize = data->callbacks->isWindowFixedSize();
        if (isFixedSize && (msg->wParam & 0xFFF0) == SC_MAXIMIZE)
        {
            *result = 0;
            return true; // 拦截最大化指令
        }
        break;
    }
    case WM_NCMOUSEMOVE:
    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONUP:
    case WM_NCLBUTTONDBLCLK:
    case WM_NCRBUTTONDOWN:
    case WM_NCRBUTTONUP:
    case WM_NCRBUTTONDBLCLK:
    case WM_NCMBUTTONDOWN:
    case WM_NCMBUTTONUP:
    case WM_NCMBUTTONDBLCLK:
    case WM_NCXBUTTONDOWN:
    case WM_NCXBUTTONUP:
    case WM_NCXBUTTONDBLCLK:
    case WM_NCMOUSEHOVER:
    {
        const WindowPart currentWindowPart = data->lastHitTestResult;
        if (WM_NCMOUSEMOVE == uMsg)
        {
            if (WindowPart::ChromeButton != currentWindowPart)
            {
                data->callbacks->resetQtGrabbedControl();
                if (data->isMouseLeaveBlocked)
                {
                    int overrideMessage = WM_NCMOUSELEAVE;
                    emulateClientAreaMessage(hWnd, uMsg, wParam, lParam, &overrideMessage);
                }
            }
            data->lastHitTestResult = WindowPart::Outside;
        }

        if (WindowPart::ChromeButton == currentWindowPart)
        {
            emulateClientAreaMessage(hWnd, uMsg, wParam, lParam);
            if (uMsg == WM_NCMOUSEMOVE)
                *result = ::DefWindowProcW(hWnd, WM_NCMOUSEMOVE, wParam, lParam);
            else
                *result = ((uMsg >= WM_NCXBUTTONDOWN) && (uMsg <= WM_NCXBUTTONDBLCLK)) ? TRUE : FALSE;
            return true;
        }
        break;
    }
    case WM_NCMOUSELEAVE:
    {
        const WindowPart currentWindowPart = data->lastHitTestResult;
        if (WindowPart::ChromeButton == currentWindowPart)
        {
            if (data->isMouseLeaveBlocked)
            {
                data->isMouseLeaveBlocked = false;
                requestForMouseLeaveMessage(hWnd, false);
            }
        }
        else
        {
            if (data->isMouseLeaveBlocked)
                emulateClientAreaMessage(hWnd, WM_NCMOUSELEAVE, wParam, lParam);
        }
        break;
    }
#if (QT_VERSION < QT_VERSION_CHECK(6, 2, 2))
    case WM_WINDOWPOSCHANGING:
    {
        const auto windowPos = reinterpret_cast<LPWINDOWPOS>(lParam);
        const QRect suggestedFrameGeometry{ windowPos->x, windowPos->y, windowPos->cx, windowPos->cy };
        const QMargins frameMargins = (QF::getWindowSystemFrameMargins(winId) + QF::getWindowCustomFrameMargins(qWindow));
        const QRect suggestedGeometry = (suggestedFrameGeometry - frameMargins);
        if (QF::toNativePixels(qWindow, qWindow->size()) != suggestedGeometry.size())
            windowPos->flags |= SWP_NOCOPYBITS;
        break;
    }
#endif
    case WM_SIZE:
    {
        break;
    }
    case WM_MOVE:
    {
        const HMONITOR currentMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        Q_ASSERT(currentMonitor);
        if (!currentMonitor || currentMonitor == data->monitor)
            break;

        data->monitor = currentMonitor;
        data->callbacks->forceChildrenRepaint();
        break;
    }
    default:
        break;
    }

    return false;
}
