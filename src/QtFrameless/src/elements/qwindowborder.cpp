#include "frameless/elements/qwindowborder.h"

#include "frameless/utils/qframelessstyle_p.h"
#include "frameless/utils/qframelessutils_p.h"

#include <QColor>
#include <QLineF>
#include <QPainter>
#include <QPainterPath>
#include <QSize>

constexpr int kMaximumBorderThickness = 100;

class QWindowBorderPrivate
{
    Q_DECLARE_PUBLIC(QWindowBorder)
private:
    explicit QWindowBorderPrivate(QWindowBorder *q);

    QWindowBorder *q_ptr;

    int thickness { QF::kDefaultWindowBordeThicknessr };
    Qt::Edges edges;
    QColor activeColor;
    QColor inactiveColor;
};

QWindowBorder::QWindowBorder(QObject *parent)
    : QObject(parent)
    , d_ptr(new QWindowBorderPrivate(this))
{

}

QWindowBorder::~QWindowBorder()
{
    delete d_ptr;
}

int QWindowBorder::thickness() const
{
    Q_D(const QWindowBorder);
    return d->thickness;
}

void QWindowBorder::setThickness(int value)
{
    Q_ASSERT(value >= 0);
    Q_ASSERT(value <= kMaximumBorderThickness);
    value = qBound(0, value, kMaximumBorderThickness);

    Q_D(QWindowBorder);
    if (value == d->thickness)
        return;

    d->thickness = value;
    emit thicknessChanged();
    emit shouldRepaint();
}

void QWindowBorder::setActiveColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid())
        return;

    Q_D(QWindowBorder);
    if (value == d->activeColor)
        return;

    d->activeColor = value;
    emit activeColorChanged();
    emit shouldRepaint();
}

void QWindowBorder::setInactiveColor(const QColor &value)
{
    Q_ASSERT(value.isValid());
    if (!value.isValid())
        return;

    Q_D(QWindowBorder);
    if (value == d->inactiveColor)
        return;

    d->inactiveColor = value;
    emit inactiveColorChanged();
    emit shouldRepaint();
}

QColor QWindowBorder::activeColor() const
{
    Q_D(const QWindowBorder);
    return d->activeColor;
}

QColor QWindowBorder::inactiveColor() const
{
    Q_D(const QWindowBorder);
    return d->inactiveColor;
}

void QWindowBorder::paint(QPainter *painter, const QSize &size, bool isActive, double radius)
{
    Q_ASSERT(nullptr != painter);
    Q_ASSERT(!size.isEmpty());
    if (nullptr == painter || size.isEmpty())
        return;

    Q_D(QWindowBorder);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QList<QLineF> lines = {};
#else
    QVector<QLineF> lines = {};
#endif

    qreal gap = d->thickness <= 1 ? 0.5 : 0;
    const auto leftTop = QPointF{ gap, gap + 1 };
    const auto rightTop = QPointF{ qreal(size.width()) - gap, leftTop.y() };
    const auto rightBottom = QPointF{ rightTop.x(), qreal(size.height()) - gap };
    const auto leftBottom = QPointF{ leftTop.x(), rightBottom.y() };
    if (d->edges & Qt::LeftEdge)
        lines.append({ leftBottom, leftTop });

    if (d->edges & Qt::TopEdge)
        lines.append({ leftTop, rightTop });

    if (d->edges & Qt::RightEdge)
        lines.append({ rightTop, rightBottom });

    if (d->edges & Qt::BottomEdge)
        lines.append({ rightBottom, leftBottom });

    if (lines.isEmpty())
        return;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPen pen = {};
    pen.setWidth(d->thickness / 2);
    pen.setColor([d, isActive]() -> QColor {
        QColor color = {};
        if (isActive)
            color = d->activeColor;
        else
            color = d->inactiveColor;
        if (color.isValid())
            return  color;
        return isActive ? QF::kDefaultWindowBorderActiveColor : QF::kDefaultWindowBorderInactiveColor;
    }());
    painter->setPen(pen);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    QPainterPath path;
    QRectF rect(QPoint(0, 0), size);
    const double length = d->thickness / 4 - 0.5;
    path.addRoundedRect(rect.adjusted(length, length + 1, -length, -length), radius, radius);
    path.setFillRule(Qt::WindingFill);

#if 0
    painter->drawLines(lines);
#else
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
#endif
    painter->restore();
}

QWindowBorderPrivate::QWindowBorderPrivate(QWindowBorder *q)
    : q_ptr(q)
    , edges(Qt::TopEdge | Qt::LeftEdge | Qt::RightEdge | Qt::BottomEdge)
{

}
