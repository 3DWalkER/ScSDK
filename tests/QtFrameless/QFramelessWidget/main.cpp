#include "frameless/widgets/qframelesswidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QFramelessWidget w;
	w.show();

	return a.exec();
}