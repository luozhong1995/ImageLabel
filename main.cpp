#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QIODevice>
#include<QMessageBox>
#include<QTextStream>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    w.setWindowTitle("engine-screenshot");
    w.show();

    return app.exec();
}
