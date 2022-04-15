#include <QApplication>
#include <QStyle>
#include <QObject>
#include "ui/MainWindow.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    pvui::MainWindow window;
    window.show();

    return app.exec();
}
