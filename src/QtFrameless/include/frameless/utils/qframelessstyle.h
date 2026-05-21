#ifndef QFRAMELESSSTYLE_H
#define QFRAMELESSSTYLE_H

#include "frameless/utils/qframelessglobal.h"
#include <QObject>

class QMargins;
class QFramelessStylePrivate;

class SC_FRAMELESS_EXPORT QFramelessStyle : public QObject
{
    Q_OBJECT
public:
    void release();

    int titleBarNormalHeight() const;
    int titleBarExtendedHeight() const;
    int titleBarContentsMargin() const;

    QFont titleBarFont() const;
    QSize windowIconSize() const;

    bool isTitleBarUsingQss() const;
    QColor titleBarActiveBackgroundColor() const;
    QColor titleBarInactiveBackgroundColor() const;
    QColor titleBarActiveForegroundColor() const;
    QColor titleBarInactiveForegroundColor() const;

    int buttonWidth() const;
    QColor buttonActiveForegroundColor() const;
    QColor buttonInactiveForegroundColor() const;
    QColor closeButtonNormalColor() const;
    QColor closeButtonHoverColor() const;
    QColor closeButtonPressColor() const;

    bool isWindowBorderEnabled() const;
    int windowBorderThickness() const;
    QColor windowBorderActiveColor() const;
    QColor windowBorderInactiveColor() const;

    QMargins contentsMargins() const;

    static QFramelessStyle *globalInstance();
    static QFramelessStyle *instance(QWidget *widget);
    static QFramelessStyle *instanceForWindow(QWidget *container);

public slots:
    void setTitleBarNormalHeight(int height);
    void setTitleBarExtendedHeight(int height);

    void setTitleBarUsingQss(bool on);

    void setButtonWidth(int width);

    void setWindowBorderEnabled(bool on);
    void setWindowBorderThickness(int thickness);
    void setWindowBorderActiveColor(const QColor &color);
    void setWindowBorderInactiveColor(const QColor &color);
    void updateWindowBorderByActiveState();

signals:
    void titleBarNormalHeightChanged(int height);
    void titleBarExtendedHeightChanged(int height);

    void titleBarUsingQssChanged(bool on);

    void buttonWidthChanged(int width);

    void windowBorderEnabledChanged(bool on);
    void windowBorderThicknessChanged(int thickness);
    void windowBorderActiveColorChanged(const QColor &color);
    void windowBorderInactiveColorChanged(const QColor &color);

private:
    explicit QFramelessStyle(QWidget *parent = nullptr);
    QFramelessStyle(QWidget *parent, QFramelessStyle *target);
    ~QFramelessStyle() override;

    QFramelessStylePrivate *d_ptr;
    Q_DECLARE_PRIVATE(QFramelessStyle)
};

#endif // QFRAMELESSSTYLE_H
