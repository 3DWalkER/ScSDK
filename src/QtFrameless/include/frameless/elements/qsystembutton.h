#ifndef QSYSTEMBUTTON_H
#define QSYSTEMBUTTON_H

#include "frameless/utils/qframelessglobal.h"
#include <QPushButton>

class QSystemButtonPrivate;

class SC_FRAMELESS_EXPORT QSystemButton : public QPushButton
{
    Q_OBJECT
public:
    enum Type
    {
        Invalid,
        WindowIcon,
        Help,
        Minimize,
        Maximize,
        Restore,
        Close
    };
    Q_ENUM(Type)

    explicit QSystemButton(Type type = Close, QWidget *parent = nullptr);
    ~QSystemButton() override;

    bool isActive() const;
    QSystemButton::Type buttonType() const;

    QColor hoverColor() const;
    QColor pressColor() const;
    QColor normalColor() const;
    QColor activeForegroundColor() const;
    QColor inactiveForegroundColor() const;

    bool isUsingQss() const;

public slots:
    void setActive(bool on);
    void setButtonType(Type type);
    void setGlyph(const QString &glyph);

    void setHoverColor(const QColor &color);
    void setPressColor(const QColor &color);
    void setNormalColor(const QColor &color);
    void setActiveForegroundColor(const QColor &color);
    void setInactiveForegroundColor(const QColor &color);

    void setUsingQss(bool on);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QSystemButtonPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QSystemButton)
};

#endif // QSYSTEMBUTTON_H
