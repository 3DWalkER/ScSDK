#include "frameless/utils/qframelessutils_p.h"

#include "frameless/utils/qsystemversion_p.h"
#include "frameless/utils/qsysapiloader_p.h"
#include "frameless/utils/qframelessstyle_p.h"

#include <Windows.h>
#include <d2d1.h>
#include <dwmapi.h>
#include <cmath>
#include <QWindow>
#include <QGuiApplication>
#include <QtGui/qpa/qplatformwindow.h>
#if (QT_VERSION < QT_VERSION_CHECK(6, 2, 0))
#   include <QtGui/qpa/qplatformnativeinterface.h>
#else
#   include <QtGui/qpa/qplatformwindow_p.h>
#endif
#include <array>
#include <private/qwinregistry_p.h>
#include <QDebug>

#ifndef SM_CYPADDEDBORDER
#   define SM_CYPADDEDBORDER SM_CXPADDEDBORDER
#endif

DECLARE_HANDLE(_DPI_AWARENESS_CONTEXT);

#ifndef _DPI_AWARENESS_CONTEXT_UNAWARE
#  define _DPI_AWARENESS_CONTEXT_UNAWARE (reinterpret_cast<_DPI_AWARENESS_CONTEXT>(-1))
#endif

#ifndef _DPI_AWARENESS_CONTEXT_SYSTEM_AWARE
#  define _DPI_AWARENESS_CONTEXT_SYSTEM_AWARE (reinterpret_cast<_DPI_AWARENESS_CONTEXT>(-2))
#endif

#ifndef _DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE
#  define _DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE (reinterpret_cast<_DPI_AWARENESS_CONTEXT>(-3))
#endif

#ifndef _DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#  define _DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 (reinterpret_cast<_DPI_AWARENESS_CONTEXT>(-4))
#endif

#ifndef _DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED
#  define _DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED (reinterpret_cast<_DPI_AWARENESS_CONTEXT>(-5))
#endif

static constexpr const char kDwmApi[] = "dwmapi";

Q_DECLARE_METATYPE(QMargins)

static constexpr const char kQtWindowCustomMarginsVar[] = "_q_windowsCustomMargins";

QF_BEGIN_NAMESPACE

typedef enum _PROCESS_DPI_AWARENESS
{
    _PROCESS_DPI_UNAWARE = 0,
    _PROCESS_SYSTEM_DPI_AWARE = 1,
    _PROCESS_PER_MONITOR_DPI_AWARE = 2,
    _PROCESS_PER_MONITOR_V2_DPI_AWARE = 3,
    _PROCESS_DPI_UNAWARE_GDISCALED = 4
} _PROCESS_DPI_AWARENESS;

enum class DpiAwareness : quint8
{
    Unknown,
    Unaware,
    System,
    PerMonitor,
    PerMonitorVersion2,
    Unaware_GdiScaled
};

static constexpr const std::array<ULONG_PTR, 10> g_registryKeyMap =
{
    0x80000000,     // HKEY_CLASSES_ROOT
    0x80000001,     // HKEY_CURRENT_USER
    0x80000002,     // HKEY_LOCAL_MACHINE
    0x80000003,     // HKEY_USERS
    0x80000004,     // HKEY_PERFORMANCE_DATA
    0x80000005,     // HKEY_CURRENT_CONFIG
    0x80000006,     // HKEY_DYN_DATA
    0x80000007,     // HKEY_CURRENT_USER_LOCAL_SETTINGS
    0x80000050,     // HKEY_PERFORMANCE_TEXT
    0x80000060      // HKEY_PERFORMANCE_NLSTEXT
};

class QRegistryKey
{
public:
    enum RegistryRootKey
    {
        ClassesRoot,
        CurrentUser,
        LocalMachine,
        Users,
        PerformanceData,
        CurrentConfig,
        DynData,
        CurrentUserLocalSettings,
        PerformanceText,
        PerformanceNlsText
    };

    explicit QRegistryKey(RegistryRootKey root, const QString key);
    ~QRegistryKey() = default;

    bool isValid() const { return m_pRegistryKey->isValid(); }

    QVariant value(const QString &name) const;
    static QVariant value(RegistryRootKey root, const QString key, const QString &name);

    static QString dwmRegistryKey() { return R"(Software\Microsoft\Windows\DWM)"; }
    static QString personalizeRegistryKey() { return R"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)"; }

    static QString dwmColorKeyName() { return R"(ColorPrevalence)"; }

private:
    std::unique_ptr<QWinRegistryKey> m_pRegistryKey;
};

QRegistryKey::QRegistryKey(QRegistryKey::RegistryRootKey root, const QString key)
{
    Q_ASSERT(!key.isEmpty());
    if (key.isEmpty())
        return;

    m_pRegistryKey = std::make_unique<QWinRegistryKey>(reinterpret_cast<HKEY>(g_registryKeyMap.at(static_cast<quint8>(root))), key);
    if (!isValid())
        m_pRegistryKey.reset();
}

QVariant QRegistryKey::value(const QString &name) const
{
    Q_ASSERT(isValid());
    Q_ASSERT(!name.isEmpty());
    if (!isValid() || name.isEmpty())
        return {};

#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    return m_pRegistryKey->value(name);
#else // (QT_VERSION < QT_VERSION_CHECK(6, 5, 0))
    const QPair<DWORD, bool> dwVal = m_pRegistryKey->dwordValue(name);
    if (dwVal.second)
        return qulonglong(dwVal.first);

    const QString strVal = m_pRegistryKey->stringValue(name);
    if (!strVal.isEmpty())
        return strVal;
    return {};
#endif
}

QVariant QRegistryKey::value(QRegistryKey::RegistryRootKey root, const QString key, const QString &name)
{
    QRegistryKey registry(root, key);
    if (!registry.isValid())
        return {};
    return registry.value(name);
}

DpiAwareness getDpiAwarenessForCurrentProcess(bool* highest = nullptr);

QColor dwmColorizationColor(bool* opaque = nullptr, bool* ok = nullptr);

QColor dwmColorizationColor(bool* opaque, bool* ok)
{
    const auto resultFromRegistry = []() -> QColor {
            const QRegistryKey registry(QRegistryKey::CurrentUser, QRegistryKey::dwmRegistryKey());
            if (!registry.isValid())
            return kDefaultDarkGrayColor;

            const QVariant value = registry.value("ColorizationColor");
            if (!value.isValid())
            return kDefaultDarkGrayColor;
            return QColor::fromRgba(qvariant_cast<DWORD>(value));
};

    if (!QSysApiLoader::isAvailable(kDwmApi, "DwmGetColorizationColor"))
    {
        if (ok)
            *ok = false;
        return resultFromRegistry();
    }

    DWORD color = 0;
    BOOL bOpaque = FALSE;
    const HRESULT hr = QSysApiLoader::get<decltype(&DwmGetColorizationColor)>(kDwmApi, "DwmGetColorizationColor")(&color, &bOpaque);
    if (FAILED(hr))
    {
        if (ok)
            *ok = false;
        return resultFromRegistry();
    }

    if (opaque)
        *opaque = bOpaque != FALSE;

    if (ok)
        *ok = true;
    return QColor::fromRgba(color);
}

bool isWindowAttachedToEdge(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return false;

    const auto hwnd = reinterpret_cast<HWND>(windowId);
    RECT windowRect;
    if (!GetWindowRect(hwnd, &windowRect))
        return false;

    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFOEX monitorInfo { };
    if (!GetMonitorInfo(hMonitor, &monitorInfo))
        return false;

    const RECT &monitorRect = monitorInfo.rcMonitor;
    return windowRect.left == monitorRect.left || windowRect.top == monitorRect.top
            || windowRect.right == monitorRect.right || windowRect.bottom == monitorRect.bottom;
}

bool isHighContrastModeEnabled()
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    if (::SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, FALSE) == FALSE)
        return false;
    return (hc.dwFlags & HCF_HIGHCONTRASTON);
}

bool shouldAppsUseDarkMode()
{
    if (!QSystemVersion::isWin10RS2OrGreater() || isHighContrastModeEnabled())
        return false;

    if (QSystemVersion::isWin10RS5OrGreater() && !QSystemVersion::isWin1019H1OrGreater())
    {
        if (QSystemVersion::isWin10OrGreater())
            return false;

        typedef BOOL(WINAPI* ShouldAppsUseDarkModePtr)(VOID);
        static const auto pShouldAppsUseDarkMode = reinterpret_cast<ShouldAppsUseDarkModePtr>(
                    QSysApiLoader::resolve("uxtheme", MAKEINTRESOURCEA(132)));
        return pShouldAppsUseDarkMode ? pShouldAppsUseDarkMode() : false;
    }

    const QRegistryKey registry(QRegistryKey::CurrentUser, QRegistryKey::personalizeRegistryKey());
    if (!registry.isValid())
        return false;

    const QVariant value = registry.value("AppsUseLightTheme");
    return value.isNull() ? false : 0 == value.toInt() ? true : false;
} 

template <typename T>
bool getWindowDwmAttribute(const WId windowId, DWORD dwAttribute, T &attribute)
{
    if (!QSysApiLoader::isAvailable(kDwmApi, "DwmGetWindowAttribute"))
        return false;

    using DwmGetWindowAttributePtr = HRESULT (WINAPI *)(HWND, DWORD, _Out_writes_bytes_(cbAttribute) PVOID, DWORD);
    auto pFunc = QSysApiLoader::get<DwmGetWindowAttributePtr>(kDwmApi, "DwmGetWindowAttribute");

    const HWND hwnd = reinterpret_cast<HWND>(windowId);
    auto hr = pFunc(hwnd, dwAttribute, &attribute, sizeof(T));
    if (SUCCEEDED(hr))
        return false;
    return true;
}

template <typename T>
bool setWindowDwmAttribute(const WId windowId, DWORD dwAttribute, const T &attribute)
{
    if (!QSysApiLoader::isAvailable(kDwmApi, "DwmSetWindowAttribute"))
        return false;

    using DwmSetWindowAttributePtr = HRESULT (WINAPI *)(HWND, DWORD, _In_reads_bytes_(cbAttribute) LPCVOID, DWORD);
    auto pFunc = QSysApiLoader::get<DwmSetWindowAttributePtr>(kDwmApi, "DwmSetWindowAttribute");

    const HWND hwnd = reinterpret_cast<HWND>(windowId);
    auto hr = pFunc(hwnd, dwAttribute, &attribute, sizeof(T));
    if (SUCCEEDED(hr))
        return false;
    return true;
}

typedef enum
{
    _DWMWCP_DEFAULT                                 = 0,
    _DWMWCP_DONOTROUND                              = 1,
    _DWMWCP_ROUND                                   = 2,
    _DWMWCP_ROUNDSMALL                              = 3
} _DWM_WINDOW_CORNER_PREFERENCE;

static constexpr DWORD _DWMWA_WINDOW_CORNER_PREFERENCE = 33;

int windowCornerRadius(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return 0;

    if (!QSystemVersion::isWin11OrGreater())
        return 0;

    _DWM_WINDOW_CORNER_PREFERENCE pref = _DWMWCP_DEFAULT;
    if (!getWindowDwmAttribute(windowId, _DWMWA_WINDOW_CORNER_PREFERENCE, pref))
        return 0;

    switch (pref)
    {
    case _DWMWCP_DONOTROUND:
        return 0;
    case _DWMWCP_ROUND:
        return 8;
    case _DWMWCP_ROUNDSMALL:
        return 0;
    default:
        return 8;
    }
}

QColor accentColor()
{
    const QColor alternative = dwmColorizationColor();
    const QRegistryKey registry(QRegistryKey::CurrentUser, QRegistryKey::dwmRegistryKey());
    if (!registry.isValid())
        return alternative;

    const QVariant value = registry.value("AccentColor");
    if (value.isNull())
        return alternative;

    const QColor abgr = QColor::fromRgba(qvariant_cast<DWORD>(value));
    if (!abgr.isValid())
        return alternative;
    return QColor(abgr.blue(), abgr.green(), abgr.red(), abgr.alpha());
}

QFont systemDefaultFont()
{
    NONCLIENTMETRICSW ncm = { 0 };
    ncm.cbSize = sizeof(NONCLIENTMETRICSW);
    if (!QSystemVersion::isWinVistaOrGreater())
        ncm.cbSize -= sizeof(WCHAR);

    QFont font;
    font.setPointSize(9);
    QString family;
    if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0))
        family = "Microsoft YaHei UI";
    else
        family = QString::fromWCharArray(ncm.lfCaptionFont.lfFaceName);
    font.setFamily(family);
    return font;
}

bool isValidWindow(const WId windowId, const bool checkVisible, const bool checkTopLevel)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return false;

    const auto hwnd = reinterpret_cast<HWND>(windowId);
    if (FALSE == ::IsWindow(hwnd))
        return false;

    const LONG_PTR styles = ::GetWindowLongPtrW(hwnd, GWL_STYLE);
    if ((styles == 0) || (styles & WS_DISABLED))
        return false;

    const LONG_PTR exStyles = ::GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if ((exStyles == 0) || (exStyles & WS_EX_TOOLWINDOW))
        return false;

    RECT rect = { 0, 0, 0, 0 };
    if (::GetWindowRect(hwnd, &rect) == FALSE)
        return false;

    if ((rect.left >= rect.right) || (rect.top >= rect.bottom))
        return false;

    if (checkVisible)
    {
        if (::IsWindowVisible(hwnd) == FALSE)
            return false;
    }

    if (checkTopLevel)
    {
        if (::GetAncestor(hwnd, GA_ROOT) != hwnd)
            return false;
    }
    return true;
}

bool isWindowFrameBorderVisible()
{
    static const auto result = []() -> bool {
            return QSystemVersion::isWin11OrGreater();
}();
    return result;
}

bool isDwmCompositionEnabled()
{
    return true;
}

bool isFullScreen(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return false;

    const auto hwnd = reinterpret_cast<HWND>(windowId);
    RECT windowRect = {};
    if (::GetWindowRect(hwnd, &windowRect) == FALSE)
        return false;

    MONITORINFOEXW mi;
    if (!getMonitorForWindow(hwnd, mi))
        return false;
    return ((windowRect.left == mi.rcMonitor.left) && (windowRect.top == mi.rcMonitor.top)
            && (windowRect.right == mi.rcMonitor.right) && (windowRect.bottom == windowRect.bottom));
}

bool isWindowNoState(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return false;

    const auto hwnd = reinterpret_cast<HWND>(windowId);
#if 0
    WINDOWPLACEMENT wp;
    SecureZeroMemory(&wp, sizeof(wp));
    wp.length = sizeof(wp);
    if (FALSE == ::GetWindowPlacement(hwnd, &wp))
        return false;
    return ((wp.showCmd == SW_NORMAL) || (wp.showCmd == SW_RESTORE));
#else
    ::SetLastError(ERROR_SUCCESS);
    const auto style = static_cast<DWORD>(::GetWindowLongPtrW(hwnd, GWL_STYLE));
    if (0 == style)
        return false;
    return (!(style & (WS_MINIMIZE | WS_MAXIMIZE)));
#endif
}

static constexpr DWORD _DWMWA_BORDER_COLOR = _DWMWA_WINDOW_CORNER_PREFERENCE + 1;

inline void setWindowBorderColor(const WId windowId, COLORREF color) {
    setWindowDwmAttribute(windowId, _DWMWA_BORDER_COLOR, color);
}

void setWindowBorderColor(const WId windowId, const QColor &color)
{
    setWindowBorderColor(windowId, RGB(color.red(), color.green(), color.blue()));
}

void setWindowBorderVisible(const WId windowId, const bool isVisible)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return;

    if (!QSystemVersion::isWin11OrGreater())
        return;

    const COLORREF noBorder = isVisible ? DWMWA_COLOR_DEFAULT : DWMWA_COLOR_NONE;
    setWindowBorderColor(windowId, noBorder);
}

quint32 getTitleBarHeight(const WId windowId, const bool scaled)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return 0;
    return (getCaptionBarHeight(windowId, scaled) + getResizeBorderThickness(windowId, false, scaled));
}

quint32 getCaptionBarHeight(const WId windowId, const bool scaled)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return 0;
    return getSystemMetrics(windowId, SM_CYCAPTION, false, scaled);
}

quint32 getResizeBorderThickness(const WId windowId, const bool horizontal, const bool scaled)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return 0;

    if (horizontal)
        return (getSystemMetrics(windowId, SM_CXSIZEFRAME, true, scaled) + getSystemMetrics(windowId, SM_CXPADDEDBORDER, true, scaled));
    else
        return (getSystemMetrics(windowId, SM_CYSIZEFRAME, false, scaled) + getSystemMetrics(windowId, SM_CYPADDEDBORDER, false, scaled));
}

int getSystemMetricsForDpi(const int nIndex, const quint32 dpi)
{
    Q_ASSERT(nIndex >= 0);
    Q_ASSERT(dpi != 0);
    if (nIndex < 0 || 0 == dpi)
    {
        ::SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    if (!QSystemVersion::isWin10OrGreater())
    {
        ::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return 0;
    }

    typedef int (WINAPI* GetSystemMetricsForDpiPtr)(int, UINT);
    static const auto pGetSystemMetricsForDpi = []() ->GetSystemMetricsForDpiPtr {
            // since Win10 1607
            GetSystemMetricsForDpiPtr pFunc = reinterpret_cast<GetSystemMetricsForDpiPtr>(
                QSysApiLoader::resolve("user32", "GetSystemMetricsForDpi"));
            if (nullptr != pFunc)
            return pFunc;

            // Win10 1507 & 1511
            pFunc = reinterpret_cast<GetSystemMetricsForDpiPtr>(
                QSysApiLoader::resolve("user32", "GetDpiMetrics"));
            return pFunc;
}();

    if (!pGetSystemMetricsForDpi)
    {
        ::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return 0;
    }

    return pGetSystemMetricsForDpi(nIndex, dpi);
}

int getSystemMetricsForDpi(const int index, const quint32 dpi, const bool horizontal)
{
    if (0 == dpi)
        return 0;

    int result = getSystemMetricsForDpi(index, dpi);
    if (result > 0)
        return result;

    static constexpr const auto defaultDpi = qreal(USER_DEFAULT_SCREEN_DPI);
    const qreal currentDpi = getPrimaryScreenDpi(horizontal) / defaultDpi;
    const qreal requestedDpi = dpi / defaultDpi;
    return std::round(qreal(::GetSystemMetrics(index)) / currentDpi * requestedDpi);
}

int getSystemMetrics(const WId windowId, const int index, const bool horizontal, const bool scaled)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return 0;

    const UINT realDpi = getWindowDpi(windowId, horizontal);
    const UINT dpi = (scaled ? realDpi : USER_DEFAULT_SCREEN_DPI);
    const int result = getSystemMetricsForDpi(index, dpi);
    if (result > 0)
        return result;

    const qreal dpr = (scaled ? qreal(1) : (qreal(realDpi) / qreal(USER_DEFAULT_SCREEN_DPI)));
    return std::round(qreal(::GetSystemMetrics(index)) / dpr);
}

bool isTitleBarColorized()
{
    if (!QSystemVersion::isWin10OrGreater())
        return false;
    const DwmColorizationArea area = getDwmColorizationArea();
    return ((area == DwmColorizationArea::TitleBar_WindowBorder) || (area == DwmColorizationArea::All));
}

DwmColorizationArea getDwmColorizationArea()
{
    if (!QSystemVersion::isWin10OrGreater())
        return DwmColorizationArea::None;

    const QVariant themeRegistry = QRegistryKey::value(
                QRegistryKey::CurrentUser, QRegistryKey::personalizeRegistryKey(), QRegistryKey::dwmColorKeyName());
    const DWORD themeValue = themeRegistry.isNull() ? 0 : themeRegistry.toInt();
    const bool theme = 0 != themeValue;

    const QVariant dwmRegistry = QRegistryKey::value(
                QRegistryKey::CurrentUser, QRegistryKey::dwmRegistryKey(), QRegistryKey::dwmColorKeyName());
    const DWORD dwmeValue = dwmRegistry.isNull() ? 0 : dwmRegistry.toInt();
    const bool dwm = 0 != dwmeValue;

    if (theme && dwm)
        return DwmColorizationArea::All;
    else if (theme)
        return DwmColorizationArea::StartMenu_TaskBar_ActionCenter;
    else if (dwm)
        return DwmColorizationArea::TitleBar_WindowBorder;
    return DwmColorizationArea::None;
}

quint32 getWindowDpi(const HWND hWnd)
{
    Q_ASSERT(hWnd);
    if (!hWnd)
    {
        ::SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    if (!QSystemVersion::isWin10OrGreater())
    {
        ::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return 0;
    }

    typedef UINT(WINAPI* GetDpiForWindowPtr)(HWND);
    static const auto pGetDpiForWindow = []() -> GetDpiForWindowPtr {
            // since Win10 1607
            auto pFunc = QSysApiLoader::get<GetDpiForWindowPtr>("user32", "GetDpiForWindow");
            if (nullptr != pFunc)
            return pFunc;

            pFunc = QSysApiLoader::get<GetDpiForWindowPtr>("user32", "GetWindowDPI");
            if (nullptr != pFunc)
            return pFunc;

            // since Win10 1607
            pFunc = QSysApiLoader::get<GetDpiForWindowPtr>("user32", MAKEINTRESOURCEA(2707));
            if (nullptr != pFunc)
            return pFunc;

            return nullptr;
}();

    if (!pGetDpiForWindow)
    {
        ::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return 0;
    }

    return pGetDpiForWindow(hWnd);
}

quint32 getWindowDpi(const WId windowId, const bool isHor)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return USER_DEFAULT_SCREEN_DPI;

    const auto hwnd = reinterpret_cast<HWND>(windowId);
    if (const UINT dpi = getWindowDpi(hwnd))
        return dpi;
    return getPrimaryScreenDpi(isHor);
}

#define _MDT_EFFECTIVE_DPI 0

quint32 getPrimaryScreenDpi(bool isHor)
{
    if (const HMONITOR hMonitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY))
    {
        // Windows 8 and onwards
        if (QSysApiLoader::isAvailable("shcore", "GetDpiForMonitor"))
        {
            UINT dpiX = 0, dpiY = 0;
            typedef HRESULT(WINAPI* GetDpiForWindowPtr)(HMONITOR, UINT, UINT*, UINT*);
            const HRESULT hr = QSysApiLoader::get<GetDpiForWindowPtr>("shcore", "GetDpiForMonitor")(hMonitor, _MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
            if (SUCCEEDED(hr) && dpiX > 0 && dpiY > 0)
                return isHor ? dpiX : dpiY;
        }

        // Windows 8 and onwards
        if (QSysApiLoader::isAvailable("shcore", "GetScaleFactorForMonitor"))
        {
            typedef HRESULT(WINAPI* GetScaleFactorForMonitorPtr)(HMONITOR, UINT*);
            UINT factor = 0;
            const HRESULT hr = QSysApiLoader::get<GetScaleFactorForMonitorPtr>("shcore", "GetScaleFactorForMonitor")(hMonitor, &factor);
            if (SUCCEEDED(hr) && 0 != factor)
                return quint32(std::round(qreal(USER_DEFAULT_SCREEN_DPI) * qreal(factor) / qreal(100)));
        }

        // Windows 2000 and onwards
        MONITORINFOEXW monitorInfo;
        SecureZeroMemory(&monitorInfo, sizeof(monitorInfo));
        monitorInfo.cbSize = sizeof(monitorInfo);
        if (FALSE != ::GetMonitorInfoW(hMonitor, &monitorInfo))
        {
            if (const HDC hdc = ::CreateDCW(monitorInfo.szDevice, monitorInfo.szDevice, nullptr, nullptr))
            {
                const int dpiX = ::GetDeviceCaps(hdc, LOGPIXELSX);
                const int dpiY = ::GetDeviceCaps(hdc, LOGPIXELSY);
                ::DeleteDC(hdc);
                if (dpiX > 0 && dpiY > 0)
                    return isHor ? dpiX : dpiY;
            }
        }
    }

    // Windows 7 and onwards
    if (QSysApiLoader::isAvailable("d2d1", "D2D1CreateFactory"))
    {
        typedef HRESULT(WINAPI* D2D1CreateFactoryPtr)(D2D1_FACTORY_TYPE, REFIID, CONST D2D1_FACTORY_OPTIONS*, void**);
        const auto pD2D1CreateFactory = QSysApiLoader::get<D2D1CreateFactoryPtr>("d2d1", "D2D1CreateFactory");
        ID2D1Factory* d2dFactory = nullptr;
        HRESULT hr = pD2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), nullptr, reinterpret_cast<void**>(&d2dFactory));
        if (SUCCEEDED(hr))
        {
            hr = d2dFactory->ReloadSystemMetrics();
            if (SUCCEEDED(hr))
            {
                FLOAT dpiX = FLOAT(0), dpiY = FLOAT(0);
                QT_WARNING_PUSH
                QT_WARNING_DISABLE_DEPRECATED
                d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
                QT_WARNING_POP
                if ((dpiX > FLOAT(0)) && (dpiY > FLOAT(0)))
                    return (isHor ? quint32(std::round(dpiX)) : quint32(std::round(dpiY)));
            }
        }

        if (nullptr != d2dFactory)
        {
            d2dFactory->Release();
            d2dFactory = nullptr;
        }
    }

    if (const HDC hdc = ::GetDC(nullptr))
    {
        const int dpiX = ::GetDeviceCaps(hdc, LOGPIXELSX);
        const int dpiY = ::GetDeviceCaps(hdc, LOGPIXELSY);
        ::ReleaseDC(nullptr, hdc);
        if (dpiX > 0 && dpiY > 0)
            return isHor ? dpiX : dpiY;
    }

    return USER_DEFAULT_SCREEN_DPI;
}

#define QF_API_USER_AVAILABLE(Func) QSysApiLoader::isAvailable("user32", #Func)

DpiAwareness getDpiAwarenessForCurrentProcess(bool* highest)
{
    if ((QF_API_USER_AVAILABLE(GetDpiAwarenessContextForProcess) || QF_API_USER_AVAILABLE(GetThreadDpiAwarenessContext))
            && QF_API_USER_AVAILABLE(AreDpiAwarenessContextsEqual) && QF_API_USER_AVAILABLE(GetAwarenessFromDpiAwarenessContext))
    {
        const auto context = []() -> _DPI_AWARENESS_CONTEXT {
                if (QF_API_USER_AVAILABLE(GetDpiAwarenessContextForProcess))
        {
                const HANDLE process = ::GetCurrentProcess();
                if (process)
        {
                typedef _DPI_AWARENESS_CONTEXT(WINAPI* GetDpiAwarenessContextForProcessPtr)(HANDLE);
                const auto result = QSysApiLoader::get<GetDpiAwarenessContextForProcessPtr>("user32", "GetDpiAwarenessContextForProcess")(process);
                if (result)
                return result;
    }
    }

                typedef _DPI_AWARENESS_CONTEXT(WINAPI* GetThreadDpiAwarenessContextPtr)(VOID);
                const auto result = QSysApiLoader::get<GetThreadDpiAwarenessContextPtr>("user32", "GetDpiAwarenessContextForProcess")();
                return result;
    }();

        if (!context)
            return DpiAwareness::Unknown;

        auto result = DpiAwareness::Unknown;
        typedef  BOOL(WINAPI* AreDpiAwarenessContextsEqualPtr)(_DPI_AWARENESS_CONTEXT, _DPI_AWARENESS_CONTEXT);
        AreDpiAwarenessContextsEqualPtr pAreDpiAwarenessContextsEqual = QSysApiLoader::get<AreDpiAwarenessContextsEqualPtr>("user32", "AreDpiAwarenessContextsEqual");
        if (FALSE != pAreDpiAwarenessContextsEqual(context, reinterpret_cast<_DPI_AWARENESS_CONTEXT>(-4)))
            result = DpiAwareness::PerMonitorVersion2;
        else if (FALSE != pAreDpiAwarenessContextsEqual(context, reinterpret_cast<_DPI_AWARENESS_CONTEXT>(-5)))
            result = DpiAwareness::Unaware_GdiScaled;
        else
        {
            typedef INT(WINAPI* GetAwarenessFromDpiAwarenessContextPtr)(_DPI_AWARENESS_CONTEXT);
            const auto awareness = QSysApiLoader::get<GetAwarenessFromDpiAwarenessContextPtr>("user32", "GetAwarenessFromDpiAwarenessContext")(context);
            switch (awareness)
            {
            case 0: // DPI_AWARENESS_UNAWARE
                result = DpiAwareness::Unaware;
                break;
            case 1: // DPI_AWARENESS_SYSTEM_AWARE
                result = DpiAwareness::System;
                break;
            case 2: // DPI_AWARENESS_PER_MONITOR_AWARE
                result = DpiAwareness::PerMonitor;
                break;
            case 3: // DPI_AWARENESS_PER_MONITOR_V2_AWARE
                result = DpiAwareness::PerMonitorVersion2;
                break;
            case 4: // DPI_AWARENESS_UNAWARE_GDISCALED
                result = DpiAwareness::Unaware_GdiScaled;
                break;
            default:
                break;
            }
        }

        if (highest)
            *highest = (result == DpiAwareness::PerMonitorVersion2);
        return result;
    }

    if (QSysApiLoader::isAvailable("shcore", "GetProcessDpiAwareness"))
    {
        UINT pda = 0;
        typedef HRESULT(WINAPI* GetProcessDpiAwarenessPtr)(HANDLE, UINT*);
        const HRESULT hr = QSysApiLoader::get<GetProcessDpiAwarenessPtr>("shcore", "GetProcessDpiAwareness")(nullptr, &pda);
        if (FAILED(hr))
            return DpiAwareness::Unknown;

        auto result = DpiAwareness::Unknown;
        switch (pda)
        {
        case 0: // PROCESS_DPI_UNAWARE
            result = DpiAwareness::Unaware;
            break;
        case 1: // PROCESS_SYSTEM_DPI_AWARE
            result = DpiAwareness::System;
            break;
        case 2: // PROCESS_PER_MONITOR_DPI_AWARE
            result = DpiAwareness::PerMonitor;
            break;
        case 3: // PROCESS_PER_MONITOR_V2_DPI_AWARE
            result = DpiAwareness::PerMonitorVersion2;
            break;
        case 4: // PROCESS_DPI_UNAWARE_GDISCALED
            result = DpiAwareness::Unaware_GdiScaled;
            break;
        }

        if (highest)
            *highest = (result == DpiAwareness::PerMonitor);
        return result;
    }

    if (QF_API_USER_AVAILABLE(IsProcessDPIAware))
    {
        typedef BOOL(WINAPI* IsProcessDPIAwarePtr)(VOID);
        const BOOL isAware = QSysApiLoader::get<IsProcessDPIAwarePtr>("user32", "IsProcessDPIAware")();
        const auto result = ((isAware == FALSE) ? DpiAwareness::Unaware : DpiAwareness::System);
        if (highest)
            *highest = (result == DpiAwareness::System);
        return result;
    }

    return DpiAwareness::Unknown;
}

bool tryToEnableHighestDpiAwarenessLevel()
{
    bool isHighestAlready = false;
    const DpiAwareness currentAwareness = getDpiAwarenessForCurrentProcess(&isHighestAlready);
    if (isHighestAlready)
        return true;

    if (QF_API_USER_AVAILABLE(SetProcessDpiAwarenessContext))
    {
        const auto SetProcessDpiAwarenessContext2 = [](const _DPI_AWARENESS_CONTEXT context) -> bool {
            Q_ASSERT(context);
            if (!context)
                return false;

            typedef BOOL(WINAPI* SetProcessDpiAwarenessContextPtr)(_DPI_AWARENESS_CONTEXT);
            const BOOL isOk = QSysApiLoader::get<SetProcessDpiAwarenessContextPtr>("user32", "SetProcessDpiAwarenessContext")(context);
            if (TRUE == isOk)
                return true;

            const DWORD dwError = ::GetLastError();
            if (ERROR_ACCESS_DENIED == dwError)
                return true;
            return false;
        };

        if (DpiAwareness::PerMonitorVersion2 == currentAwareness)
            return true;

        if (SetProcessDpiAwarenessContext2(_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
            return true;

        if (DpiAwareness::PerMonitor == currentAwareness)
            return true;

        if (SetProcessDpiAwarenessContext2(_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE))
            return true;

        if (DpiAwareness::System == currentAwareness)
            return true;

        if (SetProcessDpiAwarenessContext2(_DPI_AWARENESS_CONTEXT_SYSTEM_AWARE))
            return true;

        if (DpiAwareness::Unaware_GdiScaled == currentAwareness)
            return true;

        if (SetProcessDpiAwarenessContext2(_DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED))
            return true;
    }

    if (QSysApiLoader::isAvailable("shcore", "SetProcessDpiAwareness"))
    {
        const auto SetProcessDpiAwareness2 = [](const _PROCESS_DPI_AWARENESS pda) -> bool {
            typedef HRESULT(WINAPI* SetProcessDpiAwarenessPtr)(_PROCESS_DPI_AWARENESS);
            const HRESULT hr = QSysApiLoader::get<SetProcessDpiAwarenessPtr>("shcore", "SetProcessDpiAwareness")(pda);
            if (SUCCEEDED(hr))
                return true;

            if (E_ACCESSDENIED == hr)
                return true;
            return false;
        };

        if (DpiAwareness::PerMonitorVersion2 == currentAwareness)
            return true;

        if (SetProcessDpiAwareness2(_PROCESS_PER_MONITOR_V2_DPI_AWARE))
            return true;

        if (DpiAwareness::PerMonitor == currentAwareness)
            return true;

        if (SetProcessDpiAwareness2(_PROCESS_PER_MONITOR_DPI_AWARE))
            return true;

        if (DpiAwareness::System == currentAwareness)
            return true;

        if (SetProcessDpiAwareness2(_PROCESS_SYSTEM_DPI_AWARE))
            return true;

        if (DpiAwareness::Unaware_GdiScaled == currentAwareness)
            return true;

        if (SetProcessDpiAwareness2(_PROCESS_DPI_UNAWARE_GDISCALED))
            return true;
    }

    if (QF_API_USER_AVAILABLE(SetProcessDPIAware))
    {
        if (DpiAwareness::System == currentAwareness)
            return true;

        typedef BOOL(WINAPI* SetProcessDPIAwarePtr)(VOID);
        if (FALSE != QSysApiLoader::get<SetProcessDPIAwarePtr>("user32", "SetProcessDPIAware"))
            return true;
    }
    return false;
}

bool fixupChildWindowsDpiMessage(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return false;

    if (!QSystemVersion::isWin10OrGreater()
            || (QSystemVersion::isWin10RS2OrGreater() && DpiAwareness::PerMonitorVersion2 == getDpiAwarenessForCurrentProcess()))
        return true;

    const auto hwnd = reinterpret_cast<HWND>(windowId);
    if (setChildWindowDpiMessageEnabled(hwnd, TRUE))
        return true;

    return ERROR_CALL_NOT_IMPLEMENTED == ::GetLastError();
}

bool fixupDialogsDpiScaling()
{
    if (!QSystemVersion::isWin10OrGreater()
            || (QSystemVersion::isWin10RS2OrGreater()
                && DpiAwareness::PerMonitorVersion2 == getDpiAwarenessForCurrentProcess()))
        return true;

    typedef BOOL(WINAPI* EnablePerMonitorDialogScalingPtr)(VOID);
    static const auto pEnablePerMonitorDialogScaling = []() ->EnablePerMonitorDialogScalingPtr {
            // EnablePerMonitorDialogScaling() was once a public API, so we can load it by name,
            // but it got removed in Win10 1607, so we can't link to it directly.
            auto pFunc = reinterpret_cast<EnablePerMonitorDialogScalingPtr>(QSysApiLoader::resolve("user32", "EnablePerMonitorDialogScaling"));
            if (nullptr != pFunc)
            return pFunc;

            // EnablePerMonitorDialogScaling() was made private since Win10 1607.
            if (const auto pFunc = reinterpret_cast<EnablePerMonitorDialogScalingPtr>(
                QSysApiLoader::resolve("user32", MAKEINTRESOURCEA(2577)))) {
            return pFunc;
}
            return nullptr;
}();

    if (nullptr == pEnablePerMonitorDialogScaling || pEnablePerMonitorDialogScaling())
        return true;
    return false;
}

bool setChildWindowDpiMessageEnabled(const HWND hWnd, const bool on)
{
    Q_ASSERT(hWnd);
    if (nullptr == hWnd)
        return false;

    if (!QSystemVersion::isWin10OrGreater())
        return false;

    typedef BOOL(WINAPI* EnableChildWindowDpiMessagePtr)(HWND, BOOL);
    static const auto pEnableChildWindowDpiMessage = []() -> EnableChildWindowDpiMessagePtr {
            auto pFunc = QSysApiLoader::get<EnableChildWindowDpiMessagePtr>("win32u", "NtUserEnableChildWindowDpiMessage");
            if (nullptr != pFunc)
            return pFunc;

            pFunc = QSysApiLoader::get<EnableChildWindowDpiMessagePtr>("user32", "EnableChildWindowDpiMessage");
            if (nullptr != pFunc)
            return pFunc;

            pFunc = QSysApiLoader::get<EnableChildWindowDpiMessagePtr>("user32", MAKEINTRESOURCEA(2704));
            if (nullptr != pFunc)
            return pFunc;

            return nullptr;
}();
    if (!pEnableChildWindowDpiMessage)
        return false;
    return TRUE == pEnableChildWindowDpiMessage(hWnd, on);
}

quint64 getKeyState()
{
    long long result = 0;
    const auto get = [](const int virtualKey) -> bool {
        return (::GetAsyncKeyState(virtualKey) < 0);
    };

    const bool buttonSwapped = (::GetSystemMetrics(SM_SWAPBUTTON) != FALSE);
    if (get(VK_LBUTTON))
        result |= (buttonSwapped ? MK_RBUTTON : MK_LBUTTON);

    if (get(VK_RBUTTON))
        result |= (buttonSwapped ? MK_LBUTTON : MK_RBUTTON);

    if (get(VK_SHIFT))
        result |= MK_SHIFT;

    if (get(VK_CONTROL))
        result |= MK_CONTROL;

    if (get(VK_MBUTTON))
        result |= MK_MBUTTON;

    if (get(VK_XBUTTON1))
        result |= MK_XBUTTON1;

    if (get(VK_XBUTTON2))
        result |= MK_XBUTTON2;
    return result;
}

bool getMonitorForWindow(const HWND hwnd, MONITORINFOEXW& info)
{
    Q_ASSERT(hwnd);
    if (!hwnd)
        return false;

    const HMONITOR monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    if (nullptr == monitor)
        return false;

    SecureZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    return TRUE == ::GetMonitorInfo(monitor, &info);
}

QMargins getWindowSystemFrameMargins(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return {};

    const auto horizontalMargin = int(getResizeBorderThickness(windowId, true, true));
    const auto verticalMargin = int(getResizeBorderThickness(windowId, false, true));
    return QMargins{ horizontalMargin, verticalMargin, horizontalMargin, verticalMargin };
}

QMargins getWindowCustomFrameMargins(const QWindow* window)
{
    Q_ASSERT(window);
    if (!window)
        return {};

    if (QPlatformWindow* platformWindow = window->handle())
    {
        if (const auto ni = QGuiApplication::platformNativeInterface())
        {
            const QVariant marginsVar = ni->windowProperty(platformWindow, QString::fromUtf8(kQtWindowCustomMarginsVar));
            if (marginsVar.isValid() && !marginsVar.isNull())
                return qvariant_cast<QMargins>(marginsVar);
        }
        else
            qWarning().noquote() << __FUNCTION__ << "Failed to retrieve the platform native interface.";
    }
    else
        qWarning().noquote() << __FUNCTION__ << "Failed to retrieve the platform window.";

    const QVariant marginsVar = window->property(kQtWindowCustomMarginsVar);
    if (marginsVar.isValid() && !marginsVar.isNull())
        return qvariant_cast<QMargins>(marginsVar);
    return {};
}

bool setWindowThemeAttribute(const HWND hWnd, void* pvData, const quint32 cbData)
{
    Q_ASSERT(hWnd);
    Q_ASSERT(pvData);
    Q_ASSERT(cbData != 0);
    if (!hWnd || !pvData || (cbData == 0))
        return false;

    if (!QSysApiLoader::isAvailable("uxtheme", "SetWindowThemeAttribute"))
        return false;

    HRESULT hr = QSysApiLoader::get<decltype (&SetWindowThemeAttribute)>("uxtheme", "SetWindowThemeAttribute")(hWnd, WTA_NONCLIENT, pvData, cbData);
    return SUCCEEDED(hr);
}

bool setWindowThemeNonClientAttributes(const HWND hWnd, const quint32 dwMask, const quint32 dwAttributes)
{
    Q_ASSERT(hWnd);
    if (!hWnd)
        return false;

    struct WTA_OPTIONS2 {
        DWORD dwFlags;  // Values for each style option specified in the bitmask.
        DWORD dwMask;   // Bitmask for flags that are changing.
    };

    WTA_OPTIONS2 options = { };
    options.dwFlags = dwAttributes;
    options.dwMask = dwMask;
    return setWindowThemeAttribute(hWnd, &options, static_cast<quint32>(sizeof(options)));
}

bool triggerFrameChange(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return false;

    const auto hwnd = reinterpret_cast<HWND>(windowId);
    static constexpr const UINT swpFlags = (SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    if (::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, swpFlags) == FALSE)
        return false;

    static constexpr const UINT rdwFlags = (RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    if (::RedrawWindow(hwnd, nullptr, nullptr, rdwFlags) == FALSE)
        return false;
    return true;
}

bool updateInternalWindowFrameMargins(QWindow* window, const bool enable)
{
    Q_ASSERT(window);
    if (!window)
        return false;

    const WId windowId = window->winId();
    const auto margins = [enable, windowId]() -> QMargins {
        if (!enable)
            return { };

        const int titleBarHeight = getTitleBarHeight(windowId, true);
        if (!isWindowFrameBorderVisible())
        {
            const int frameSizeX = getResizeBorderThickness(windowId, true, true);
            const int frameSizeY = getResizeBorderThickness(windowId, false, true);
            return { -frameSizeX, -titleBarHeight, -frameSizeX, -frameSizeY };
        }
        else
            return { 0, -titleBarHeight, 0, 0 };
    }();
    const QVariant marginsVar = QVariant::fromValue(margins);
    window->setProperty(kQtWindowCustomMarginsVar, marginsVar);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (QPlatformWindow* platformWindow = window->handle())
    {
        if (const auto ni = QGuiApplication::platformNativeInterface())
            ni->setWindowProperty(platformWindow, kQtWindowCustomMarginsVar, marginsVar);
        else
            return false;
    }
    else
    {
        qWarning() << "Failed to retrieve the platform native interface.";
        return false;
    }
#else
    if (const auto platformWindow = dynamic_cast<QNativeInterface::Private::QWindowsWindow*>(window->handle()))
        platformWindow->setCustomMargins(margins);
    else
    {
        WARNING << "Failed to retrieve the platform window.";
        return false;
    }
#endif

    return triggerFrameChange(windowId);
}

bool updateAllDirectXSurfaces()
{
    if (!QSysApiLoader::isAvailable("dwmapi", "DwmFlush"))
        return false;

    typedef HRESULT(WINAPI* DwmFlushPtr)();
    const HRESULT hr = QSysApiLoader::get<DwmFlushPtr>("dwmapi", "DwmFlush")();
    return SUCCEEDED(hr);
}

bool hideOriginalTitleBarElements(const WId windowId, const bool disable)
{
    Q_ASSERT(windowId);
    if (!windowId)
        return 0;

    const auto hwnd = reinterpret_cast<HWND>(windowId);
    static constexpr const DWORD validBits = (WTNCA_NODRAWCAPTION | WTNCA_NODRAWICON | WTNCA_NOSYSMENU);
    const DWORD mask = (disable ? validBits : 0);
    return setWindowThemeNonClientAttributes(hwnd, mask, mask);
}

#define QF_API_WINMM_AVAILABLE(Func) QSysApiLoader::isAvailable("winmm", #Func)

bool syncWmPaintWithDwm()
{
    if (!isDwmCompositionEnabled())
        return false;

    if (!(QF_API_WINMM_AVAILABLE(timeGetDevCaps) && QF_API_WINMM_AVAILABLE(timeBeginPeriod)
          && QF_API_WINMM_AVAILABLE(timeEndPeriod) && QSysApiLoader::isAvailable("dwmapi", "DwmGetCompositionTimingInfo")))
        return false;

    LARGE_INTEGER freq = {};
    if (FALSE == ::QueryPerformanceFrequency(&freq))
        return  false;

    typedef struct _TIMECAPS {
        UINT    wPeriodMin;     /* minimum period supported  */
        UINT    wPeriodMax;     /* maximum period supported  */
    } _TIMECAPS, * _PTIMECAPS;

    typedef UINT(WINAPI* TimeGetDevCapsPtr)(_PTIMECAPS, UINT);
    _TIMECAPS tc = {};
    if (MMSYSERR_NOERROR != QSysApiLoader::get<TimeGetDevCapsPtr>("winmm", "timeGetDevCaps")(&tc, sizeof(tc)))
        return false;

    typedef UINT(WINAPI* TimeBeginPeriodPtr)(UINT);
    if (TIMERR_NOERROR != QSysApiLoader::get<TimeBeginPeriodPtr>("winmm", "timeBeginPeriod")(tc.wPeriodMin))
        return false;

    bool isSucceed = false;
    do {
        LARGE_INTEGER now0;
        if (FALSE == ::QueryPerformanceCounter(&now0))
            break;

        DWM_TIMING_INFO dti;
        SecureZeroMemory(&dti, sizeof(dti));
        dti.cbSize = sizeof(dti);
        const HRESULT hr = QSysApiLoader::get<decltype(&DwmGetCompositionTimingInfo)>("dwmapi", "DwmGetCompositionTimingInfo")(nullptr, &dti);
        if (FAILED(hr))
            break;

        LARGE_INTEGER now1;
        if (FALSE == ::QueryPerformanceCounter(&now1))
            break;

        const auto period = qreal(dti.qpcRefreshPeriod);
        const auto dt = qreal(dti.qpcVBlank - now1.QuadPart);
        const qreal ratio = (dt / period);
        auto w = dt > qreal(0) || qFuzzyIsNull(dt) ? ratio : (ratio - qreal(1));
        auto m = (dt - (period * w));
        if ((m < qreal(0)) || qFuzzyCompare(m, period) || (m > period))
            break;

        const qreal m_ms = (qreal(1000) * m / qreal(freq.QuadPart));
        ::Sleep(static_cast<DWORD>(std::round(m_ms)));

        isSucceed = true;
    } while (0);

    typedef UINT(WINAPI* TimeEndPeriod)(UINT);
    if (TIMERR_NOERROR != QSysApiLoader::get<TimeEndPeriod>("winmm", "timeEndPeriod")(tc.wPeriodMin))
        return false;

    return isSucceed;
}

QF_END_NAMESPACE
