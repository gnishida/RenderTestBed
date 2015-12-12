#include "glew.h"
#include <QApplication>
#include "LC_UrbanMain.h"

using namespace LC;

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);	
	LCUrbanMain w;
	w.showMaximized();
	return a.exec();
}