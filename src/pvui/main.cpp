#include "MainWindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QObject>
#include <QStyle>

int main(int argc, char* argv[]) {
  QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
  QCoreApplication::setOrganizationName("pView");
  QCoreApplication::setOrganizationDomain("pviewapp.github.io");
  QCoreApplication::setApplicationName("pView");
  QApplication app(argc, argv);

  pvui::MainWindow window;
  window.show();

  return app.exec();
}
