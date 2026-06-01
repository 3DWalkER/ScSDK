#ifndef QOPTIONSDIALOG_H
#define QOPTIONSDIALOG_H

#include <QDialog>

class QOptionsDialog : public QDialog
{
public:
	explicit QOptionsDialog(QWidget* parent = nullptr);
	~QOptionsDialog() override;
};

#endif // QOPTIONSDIALOG_H