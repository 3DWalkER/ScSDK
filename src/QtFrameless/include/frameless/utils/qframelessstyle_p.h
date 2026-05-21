#ifndef QFRAMELESSSTYLE_P_H
#define QFRAMELESSSTYLE_P_H

#include "qframelessstyle.h"
#include "qframelessglobal_p.h"

#include <QColor>

QF_BEGIN_NAMESPACE

/** ------------------------------------------------------------------------------
 * 基础颜色
 *------------------------------------------------------------------------------*/
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#   define kDefaultDarkGrayColor QColorConstants::DarkGray
#else // (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    constexpr QColor kDefaultDarkGrayColor = {169, 169, 169};
#endif // (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))

/** ------------------------------------------------------------------------------
 * 标题栏
 *------------------------------------------------------------------------------*/
/**
 * @brief kDefaultTitleHeight 标题栏默认高度
 */
constexpr int kDefaultTitleHeight = 22;

/**
 * @brief kDefaultTitleBarExtendedHeight 标题栏默认扩展高度
 */
constexpr int kDefaultTitleBarExtendedHeight = 30;

/**
 * @brief kDefaultTitleBarFontPointSize 标题栏默认图标尺寸
 */
constexpr QSize kDefaultWindowIconSize = { 16, 16 };

/**
 * @brief kDefaultTitleBarContentsMargin 标题栏默认约束边界
 */
constexpr int kDefaultTitleBarContentsMargin = 10;

/**
 * @brief kDefaultButtonWidth 按钮的默认宽度
 */
constexpr int kDefaultButtonWidth = 46;

/**
 * @brief kDefaultBtnNormalColor 按钮的默认背景颜色
 */
constexpr QColor kDefaultBtnNormalColor = QColorConstants::Transparent;

/**
 * @brief kDefaultBtnNormalColor 按钮的悬停默认背景颜色
 */
constexpr QColor kDefaultBtnHoverColor = { 232, 232, 232 };

/**
 * @brief kDefaultBtnPressColor 按钮的按压默认背景颜色
 */
constexpr QColor kDefaultBtnPressColor = { 235, 235, 235 };

/**
 * @brief kDefaultCloseBtnPressColor 关闭按按压停默认颜色
 */
constexpr QColor kDefaultCloseBtnPressColor = { 199, 64, 49 };

/**
 * @brief kDefaultCloseBtnHoverColor 关闭按钮悬停默认颜色
 */
constexpr QColor kDefaultCloseBtnHoverColor = { 196, 43, 28 };

/** ------------------------------------------------------------------------------
 * 窗口边框
 *------------------------------------------------------------------------------*/
/**
 * @brief kDefaultWindowBorderActiveColor 窗口边框激活状态的默认颜色
 */
constexpr QColor kDefaultWindowBorderActiveColor = { 112, 112, 112 };

/**
 * @brief kDefaultWindowBorderInactiveColor 窗口边框失活状态的默认颜色
 */
constexpr QColor kDefaultWindowBorderInactiveColor = { 170, 170, 170 };

/**
 * @brief kDefaultWindowBordeThicknessr 窗口边框默认厚度
 */
constexpr int kDefaultWindowBordeThicknessr = 1;

/**
 * @brief kDefaultResizeBorderThickness 窗口默认可拉伸区域厚度
 */
constexpr int kDefaultResizeBorderThickness = 8;

QF_END_NAMESPACE

class QFramelessStylePrivate
{
    Q_DECLARE_PUBLIC(QFramelessStyle)
public:
    explicit QFramelessStylePrivate(QFramelessStyle *q);
    QFramelessStylePrivate(QFramelessStyle *q, QWidget *widget, QFramelessStylePrivate *target);

    void refresh();
    void updateSystemWindowBorder(bool isEnabledChanged);

    QFramelessStyle *q_ptr;
    int titleNormalHeight { QF::kDefaultTitleHeight };
    int titleExtendedHeight { QF::kDefaultTitleBarExtendedHeight };
    int titleBarContentsMargin;

    bool isTitleBarUsingQss { };

    QFont titleBarFont;
    QSize windowIconSize;

    int buttonWidth { QF::kDefaultButtonWidth };

    QColor buttonTextActiveColor;
    QColor buttonTextInactiveColor;
    QColor closeButtonNormalColor;
    QColor closeButtonHoverColor;
    QColor closeButtonPressColor;

    bool isWindowBorderEnabled;
    int windowBorderThickness;
    QColor windowBorderActiveColor;
    QColor windowBorderInactiveColor;

    QWidget *widget;
    bool isGlobal { false };
    static QFramelessStyle *instance;
    static QHash<QWidget *, QFramelessStyle *> instances;
};

#endif // QFRAMELESSSTYLE_P_H
