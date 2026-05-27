#ifndef QCOLORBUTTON_H
#define QCOLORBUTTON_H

#include "qtutils/qutilsglobal.h"
#include <QPushButton>

class QColorButtonPrivate;

class QU_API_EXPORT QColorButton : public QPushButton
{
    Q_OBJECT
public:
    enum ResponsiveMode
    {
        None,
        Click,
        DoubleClick
    };

    explicit QColorButton(ResponsiveMode mode = Click, QWidget *parent = nullptr);
    ~QColorButton();

    ResponsiveMode responsiveMode() const;
    void setResponsiveMode(ResponsiveMode mode);

    QColor color() const;
    void setColor(const QColor &color);

private:
    QColorButtonPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QColorButton)
    Q_PRIVATE_SLOT(d_func(), void _q_selectColor())

protected:
    void mouseDoubleClickEvent(QMouseEvent *ev) override;

signals:
    void colorChanged(const QColor &color);
};

#endif // QCOLORBUTTON_H
