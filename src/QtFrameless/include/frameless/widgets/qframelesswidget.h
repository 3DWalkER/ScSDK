#ifndef QFRAMELESSWIDGET_H
#define QFRAMELESSWIDGET_H

#include "frameless/utils/qframelessglobal.h"
#include <QWidget>

class QWidgetsContainer;
class QFramelessWidgetPrivate;

class SC_FRAMELESS_EXPORT QFramelessWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QFramelessWidget(QWidget *parent = nullptr);
    ~QFramelessWidget() override;

    QWidgetsContainer *container() const;

private:
    QFramelessWidgetPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QFramelessWidget)
};

#endif // QFRAMELESSWIDGET_H
