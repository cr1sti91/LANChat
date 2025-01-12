#include "server_mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QPalette windowPalette(QPalette::Window, Qt::white);
    w.setPalette(windowPalette);

    w.show();
    return a.exec();
}






