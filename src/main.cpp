#include "ui/MainWindow.h"
#include <QApplication>
#include <QObject>
#include <QStyle>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  pvui::MainWindow window;
  window.show();

  return app.exec();
}
