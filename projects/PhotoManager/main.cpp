#include "PhotoManagerWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QStringList files = QApplication::arguments();
	files.removeFirst();

	PhotoManagerWindow w(files);
	w.show();
	return a.exec();
}
