#ifndef QCONFIGRADIOBUTTON_H
#define QCONFIGRADIOBUTTON_H

#include "qtutils/qutilsglobal.h"
#include <QRadioButton>

class QConfigRadioButtonPrivate;

class QU_API_EXPORT QConfigRadioButton : public QRadioButton
{
    Q_OBJECT

    Q_PROPERTY(QVariant assignedValue READ assignedValue WRITE setAssignedValue)

public:
    explicit QConfigRadioButton(QWidget *parent = nullptr);
    ~QConfigRadioButton();

    QVariant assignedValue() const;
    void setAssignedValue(const QVariant &value);

public slots:
    void alignToValue(const QVariant &value);

private:
    QConfigRadioButtonPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QConfigRadioButton)
    Q_PRIVATE_SLOT(d_func(), void _q_toggled(bool))

signals:
    void toggledOn(const QVariant &assignedValue);
    void toggledOff(const QVariant &assignedValue);
};

#endif // QCONFIGRADIOBUTTON_H
