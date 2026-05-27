#include "qtutils/buttons/qconfigradiobutton.h"

class QConfigRadioButtonPrivate
{
    Q_DECLARE_PUBLIC(QConfigRadioButton)
public:
    QConfigRadioButtonPrivate(QConfigRadioButton *q);

    void _q_toggled(bool checked);

    QConfigRadioButton *q_ptr;
    QVariant assignedValue;
    bool isHandlingSlot = false;
};

QConfigRadioButtonPrivate::QConfigRadioButtonPrivate(QConfigRadioButton *q)
    : q_ptr(q)
{

}

void QConfigRadioButtonPrivate::_q_toggled(bool checked)
{
    if (isHandlingSlot)
        return;

    Q_Q(QConfigRadioButton);
    if (checked)
        emit q->toggledOn(assignedValue);
    else
        emit q->toggledOff(assignedValue);
}


QConfigRadioButton::QConfigRadioButton(QWidget *parent)
    : QRadioButton(parent)
    , d_ptr(new QConfigRadioButtonPrivate(this))
{
    connect(this, SIGNAL(toggled(bool)), this, SLOT(_q_toggled(bool)));
}

QConfigRadioButton::~QConfigRadioButton()
{
    delete d_ptr;
}

QVariant QConfigRadioButton::assignedValue() const
{
    Q_D(const QConfigRadioButton);
    return d->assignedValue;
}

void QConfigRadioButton::setAssignedValue(const QVariant &value)
{
    Q_D(QConfigRadioButton);
    d->assignedValue = value;
}

void QConfigRadioButton::alignToValue(const QVariant &value)
{
    Q_D(QConfigRadioButton);
    d->isHandlingSlot = true;
    setChecked(value == d->assignedValue);
    d->isHandlingSlot = false;
}

#include "qtutils/buttons/moc_qconfigradiobutton.cpp"
