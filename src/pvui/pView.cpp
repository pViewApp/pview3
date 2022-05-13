#include "MainWindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QObject>
#include <QStyle>

int main(int argc, char* argv[]) {
  QCoreApplication::setOrganizationName("pView");
  QCoreApplication::setOrganizationDomain("pviewapp.github.io");
  QCoreApplication::setApplicationName("pView");
  QApplication app(argc, argv);

  pvui::MainWindow window;
  window.show();

  return app.exec();
}
