#include "mainwindow.h"
#include <QApplication>
#include <QtDebug>
#include <QSysInfo>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName("Kerimov David, Slobodan Lelikov");
    app.setApplicationName("Textr");

    MainWindow window;
    QApplication::setStyle("fusion");

    window.show();
    return app.exec();
}
