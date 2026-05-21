#ifndef QFRAMELESSDIALOG_H
#define QFRAMELESSDIALOG_H

#include "frameless/utils/qframelessglobal.h"
#include <QDialog>

class QFramelessDialogPrivate;

class SC_FRAMELESS_EXPORT QFramelessDialog : public QDialog
{
public:
    explicit QFramelessDialog(QWidget *parent = nullptr);
    ~QFramelessDialog() override;

    int exec() override;

private:
    QFramelessDialogPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QFramelessDialog)
};

#endif // QFRAMELESSDIALOG_H
