#include "qtutils/buttons/qcolorbutton.h"

#include <QColorDialog>
#include <QResizeEvent>
#include <QDialogButtonBox>

class QColorButtonPrivate
{
    Q_DECLARE_PUBLIC(QColorButton)
public:
    QColorButtonPrivate(QColorButton *q);

    void _q_selectColor();

    QColorButton *q_ptr;
    QColor color;
    QColorButton::ResponsiveMode mode = QColorButton::None;
};

QColorButtonPrivate::QColorButtonPrivate(QColorButton *q)
    : q_ptr(q)
{

}

void QColorButtonPrivate::_q_selectColor()
{
    Q_Q(QColorButton);
    QColorDialog dialog(this->color, q);
    dialog.setWindowTitle(_QU("选择颜色"));
    dialog.setOptions(QColorDialog::ShowAlphaChannel);
    QList<QDialogButtonBox *> pBtns = dialog.findChildren<QDialogButtonBox *>(QString(), Qt::FindDirectChildrenOnly);
    if (!pBtns.isEmpty())
    {
        pBtns[0]->button(QDialogButtonBox::Ok)->setText(_QU("确定(&O)"));
        pBtns[0]->button(QDialogButtonBox::Cancel)->setText(_QU("取消(&C)"));
    }
    dialog.exec();

    QColor color = dialog.selectedColor();
    if (color.isValid())
        q->setColor(color);
}

QColorButton::QColorButton(ResponsiveMode mode, QWidget *parent)
    : QPushButton(parent)
    , d_ptr(new QColorButtonPrivate(this))
{
    setAutoFillBackground(true);
    setStyleSheet("QColorButton { border: 2px solid #000000; }");
    setResponsiveMode(mode);
}

QColorButton::~QColorButton()
{
    delete d_ptr;
}

QColorButton::ResponsiveMode QColorButton::responsiveMode() const
{
    Q_D(const QColorButton);
    return d->mode;
}

void QColorButton::setResponsiveMode(QColorButton::ResponsiveMode mode)
{
    Q_D(QColorButton);
    if (mode == d->mode)
        return;

    d->mode = mode;
    if (QColorButton::Click == mode)
        connect(this, SIGNAL(clicked(bool)), this, SLOT(_q_selectColor()));
    else
        disconnect(this, SIGNAL(clicked(bool)), this, SLOT(_q_selectColor()));
}

QColor QColorButton::color() const
{
    Q_D(const QColorButton);
    return d->color;
}

void QColorButton::setColor(const QColor &color)
{
    Q_D(QColorButton);
    if (color == d->color)
        return;

    d->color = color;
    setStyleSheet(QString("QColorButton { border: 2px solid #000000; background-color: rgba(%1, %2, %3, 255); }").arg(color.red()).arg(color.green()).arg(color.blue()));
    emit colorChanged(color);
}

void QColorButton::mouseDoubleClickEvent(QMouseEvent *ev)
{
    Q_D(QColorButton);
    if (QColorButton::DoubleClick == d->mode)
        d->_q_selectColor();
    return QPushButton::mouseDoubleClickEvent(ev);
}

#include "qtutils/buttons/moc_qcolorbutton.cpp"
