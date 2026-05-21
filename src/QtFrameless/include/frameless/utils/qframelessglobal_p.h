#ifndef QFRAMELESSGLOBAL_P_H
#define QFRAMELESSGLOBAL_P_H

#include "frameless/elements/qsystembutton.h"

#include <QColor>
#include <QPointer>
#include <qwindowdefs.h>

#define QF_BEGIN_NAMESPACE namespace QF {
#define QF_END_NAMESPACE }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    using QT_NATIVE_EVENT_RESULT_TYPE = qintptr;
#else
    using QT_NATIVE_EVENT_RESULT_TYPE = long;
#endif

constexpr char kSysMenuDisableMoveVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_DISABLE_MOVE";
constexpr char kSysMenuDisableSizeVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_DISABLE_SIZE";
constexpr char kSysMenuDisableMinimizeVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_DISABLE_MINIMIZE";
constexpr char kSysMenuDisableMaximizeVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_DISABLE_MAXIMIZE";
constexpr char kSysMenuDisableRestoreVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_DISABLE_RESTORE";
constexpr char kSysMenuDisableCloseVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_DISABLE_CLOSE";
constexpr char kSysMenuRemoveMoveVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_REMOVE_MOVE";
constexpr char kSysMenuRemoveSizeVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_REMOVE_SIZE";
constexpr char kSysMenuRemoveMinimizeVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_REMOVE_MINIMIZE";
constexpr char kSysMenuRemoveMaximizeVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_REMOVE_MAXIMIZE";
constexpr char kSysMenuRemoveRestoreVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_REMOVE_RESTORE";
constexpr char kSysMenuRemoveSeparatorVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_REMOVE_SEPARATOR";
constexpr char kSysMenuRemoveCloseVar[] = "QFRAMELESSHELPER_SYSTEM_MENU_REMOVE_CLOSE";


QF_BEGIN_NAMESPACE

enum ExtraDataType
{
    WindowsUtilities,
    LinuxUtilities,
    MacOSUtilities,
    FramelessWidgetsHelper,
    FramelessQuickHelper
};

/**
 * @brief The SystemTheme enum 系统主题
 */
enum class SystemTheme : quint8
{
    Unknown,
    Light,
    Dark,
    HighContrast
};

/**
 * @brief The Dpi struct 每英寸点数
 */
struct Dpi
{
    quint32 x = 0;
    quint32 y = 0;
};

QF_END_NAMESPACE

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
inline uint qHash(const QF::ExtraDataType key, const uint seed = 0) noexcept {
    return ::qHash(static_cast<quint8>(key), seed);
}
#endif

using GetWindowFlagsCallback = std::function<Qt::WindowFlags()>;
using SetWindowFlagsCallback = std::function<void(const Qt::WindowFlags)>;
using GetWindowIdCallback = std::function<WId()>;
using GetWindowHandleCallback = std::function<QWindow *()>;
using GetWindowSizeCallback = std::function<QSize()>;
using IsWindowFixedSizeCallback = std::function<bool()>;
using SetWindowPositionCallback = std::function<void(const QPoint &)>;
using GetWindowScreenCallback = std::function<QScreen *()>;
using GetWindowStateCallback = std::function<Qt::WindowState()>;
using SetWindowStateCallback = std::function<void(const Qt::WindowState)>;
using IsInsideSystemButtonsCallback = std::function<bool(const QPoint &, QSystemButton::Type *)>;
using IsInsideTitleBarDraggableAreaCallback = std::function<bool(const QPoint &)>;
using IsShouldIgnoreMouseEventsCallback = std::function<bool(const QPoint &)>;
using SetPropertyCallback = std::function<void(const char *, const QVariant &)>;
using GetPropertyCallback = std::function<QVariant(const char *, const QVariant &)>;
using ResetQtGrabbedControlCallback = std::function<bool()>;
using ForceChildrenRepaintCallback = std::function<void()>;
using ShowSystemMenuCallback = std::function<void(const QPoint &)>;
using SetCursorCallback = std::function<void(const QCursor &)>;
using UnsetCursorCallback = std::function<void()>;

/**
 * @brief The QFramelessCallbacks struct 无边框回调函数
 */
struct QFramelessCallbacks
{
    GetWindowFlagsCallback getWindowFlags { };
    SetWindowFlagsCallback setWindowFlags { };
    GetWindowIdCallback getWindowId { };
    GetWindowHandleCallback getWindowHandle { };
    GetWindowSizeCallback getWindowSize { };
    IsWindowFixedSizeCallback isWindowFixedSize { };
    SetWindowPositionCallback setWindowPosition { };
    GetWindowStateCallback getWindowState { };
    SetWindowStateCallback setWindowState { };
    GetWindowScreenCallback getWindowScreen { };
    IsInsideTitleBarDraggableAreaCallback isInsideTitleBarDraggableArea { };
    IsInsideSystemButtonsCallback isInsideSystemButtons { };
    IsShouldIgnoreMouseEventsCallback isShouldIgnoreMouseEvents { };
    SetPropertyCallback setProperty { };
    GetPropertyCallback getProperty { };
    ResetQtGrabbedControlCallback resetQtGrabbedControl { };
    ForceChildrenRepaintCallback forceChildrenRepaint{ };
    ShowSystemMenuCallback showSystemMenu { };
    SetCursorCallback setCursor { };
    UnsetCursorCallback unsetCursor { };

    QFramelessCallbacks(){ }
    virtual ~QFramelessCallbacks(){ }

    using PtrType = std::shared_ptr<QFramelessCallbacks>;
    static PtrType create() { return std::make_shared<QFramelessCallbacks>(); }

private:
    Q_DISABLE_COPY(QFramelessCallbacks)
};

using QFramelessCallbacksPtr = QFramelessCallbacks::PtrType;

/**
 * @brief The QFramelessExtraData struct 无边框窗口额外窗口信息基类
 */
struct QFramelessExtraData
{
    QFramelessExtraData() { }
    virtual ~QFramelessExtraData() { }

    using PtrType = std::shared_ptr<QFramelessExtraData>;

private:
    Q_DISABLE_COPY(QFramelessExtraData)
};

using QFramelessExtraDataPtr = QFramelessExtraData::PtrType;
using QFramelessExtraDataPtrs = QList<QFramelessExtraDataPtr>;
using QFramelessExtraDataHash = QHash<QF::ExtraDataType, QFramelessExtraDataPtr>;

/**
 * @brief The QFramelessData struct 无边框窗口信息
 */
struct QFramelessData
{
    QObject *window { };
    WId windowId { };
    QObject *internalEventHandler { };
    bool isFrameless { };
    QFramelessCallbacksPtr callbacks { };
    QFramelessExtraDataHash extraData { };

    QFramelessData() { }
    virtual ~QFramelessData() { }

    using PtrType = std::shared_ptr<QFramelessData>;
    static PtrType create();

private:
    Q_DISABLE_COPY(QFramelessData)
};

using QFramelessDataPtr = QFramelessData::PtrType;
using QFramelessDataPtrs = QList<QFramelessDataPtr>;
using QFramelessDataHash = QHash<QObject *, QFramelessDataPtr>;

#endif // QFRAMELESSGLOBAL_P_H
