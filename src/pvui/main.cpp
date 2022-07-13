#include "MainWindow.h"
#include "ThemeManager.h"
#include <QApplication>
#include <QCoreApplication>
#include <QLocale>

int main(int argc, char* argv[]) {
  QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setOrganizationName("pView");
  QCoreApplication::setOrganizationDomain("pviewapp.github.io");
  QCoreApplication::setApplicationName("pView");
  QApplication app(argc, argv);

  pvui::ThemeManager::initialize();

  pvui::MainWindow window;
  window.show();

  return app.exec();
}
