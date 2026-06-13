#include <qapplication.h>
#include "src/views/MainWindow.h"
#include "src/dialog/ScreenConfigDialog.h"
#include <qmessagebox.h>
#include <qfile.h>

int main(int argc, char** argv) {
    QApplication app(argc, argv);

	QString style;
	QString path = ":/styles/styles/app.qss";
	QFile resFile(path);
	if (resFile.open(QIODevice::ReadOnly)) {
		style = resFile.readAll();
		resFile.close();
		app.setStyleSheet(style);
	}
	else {
		qDebug() << "Failed to open resource stylesheet" << path << resFile.errorString();
	}
    ScreenConfigDialog screenDialog;
    screenDialog.exec();


    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}