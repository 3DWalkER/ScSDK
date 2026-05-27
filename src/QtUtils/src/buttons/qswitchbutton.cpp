#include "qtutils/buttons/qswitchbutton.h"

#include <QPainter>

class QSwitchButtonPrivate
{
    Q_DECLARE_PUBLIC(QSwitchButton)
public:
    explicit QSwitchButtonPrivate(QSwitchButton *q);

    /**
     * @brief drawBackground 绘制背景
     * @param p 画笔
     */
    void drawBackground(QPainter *p);

    QSwitchButton *q_ptr;

    QString onText, offText;
    QColor onBgColor, offBgColor;
    QColor onTextColor, offTextColor;
    QColor onSliderColor, offSliderColor;
};

QSwitchButtonPrivate::QSwitchButtonPrivate(QSwitchButton *q)
    : q_ptr(q)
    , onText (QStringLiteral("开"))
    , offText(QStringLiteral("关"))
    , onBgColor(QColor(60, 255, 100, 60))
    , offBgColor(QColor(255, 255, 255, 60))
    , onTextColor(Qt::white)
    , offTextColor(Qt::white)
    , onSliderColor(Qt::green)
    , offSliderColor(Qt::white)
{

}

void QSwitchButtonPrivate::drawBackground(QPainter *p)
{
    Q_Q(QSwitchButton);
    p->save();
    p->restore();
}


QSwitchButton::QSwitchButton(QWidget *parent)
    : QWidget(parent)
{

}

QSwitchButton::~QSwitchButton()
{
    delete d_ptr;
}

QString QSwitchButton::onText() const
{
    Q_D(const QSwitchButton);
    return d->onText;
}

void QSwitchButton::setOnText(const QString &text)
{
    Q_D(QSwitchButton);
    d->onText = text;
}

QString QSwitchButton::offText() const
{
    Q_D(const QSwitchButton);
    return d->offText;
}

void QSwitchButton::setOffText(const QString &text)
{
    Q_D(QSwitchButton);
    d->offText = text;
}

void QSwitchButton::setOnBackgroundColor(const QColor &color)
{
    Q_D(QSwitchButton);
    d->onBgColor = color;
    update();
}

void QSwitchButton::setOffBackgroundColor(const QColor &color)
{
    Q_D(QSwitchButton);
    d->offBgColor = color;
    update();
}

void QSwitchButton::paintEvent(QPaintEvent *)
{

}
