#include "qoptionsdialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QOptionsDialog d;
	d.show();

	return a.exec();
}