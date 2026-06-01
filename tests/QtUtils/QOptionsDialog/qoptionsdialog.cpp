#include "qoptionsdialog.h"

QOptionsDialog::QOptionsDialog(QWidget* parent)
	: QDialog(parent)
{
	setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
}

QOptionsDialog::~QOptionsDialog()
{
}
