#ifndef QWINDOWBORDER_H
#define QWINDOWBORDER_H

#include "frameless/utils/qframelessglobal.h"
#include <QObject>

class QPainter;
class QWindowBorderPrivate;

class SC_FRAMELESS_EXPORT QWindowBorder : public QObject
{
    Q_OBJECT
public:
    explicit QWindowBorder(QObject *parent = nullptr);
    ~QWindowBorder() override;

    int thickness() const;
    QColor activeColor() const;
    QColor inactiveColor() const;

public slots:
    void paint(QPainter *painter, const QSize &size, bool isActive, double radius);

    void setThickness(int value);
    void setActiveColor(const QColor &value);
    void setInactiveColor(const QColor &value);

signals:
    void thicknessChanged();
    void activeColorChanged();
    void inactiveColorChanged();
    void shouldRepaint();

private:
    QWindowBorderPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QWindowBorder)
};

#endif // QWINDOWBORDER_H
