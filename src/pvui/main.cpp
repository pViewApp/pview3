#include "MainWindow.h"
#include "ThemeManager.h"
#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QLocale>
#include <QObject>

namespace {
void installFallbackIcons() {
  QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << ":/icons");
  if (QIcon::themeName().isNull()) { // if there is no icon theme already set (this will be false on x11)
    QIcon::setThemeName("pview");    // dummy theme which doesn't exist
                                     // this forces Qt to use our fallback icons
  }
}
} // namespace

int main(int argc, char* argv[]) {
  QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setOrganizationName("pView");
  QCoreApplication::setOrganizationDomain("pviewapp.github.io");
  QCoreApplication::setApplicationName("pView");

  installFallbackIcons();
  QApplication app(argc, argv);

  pvui::ThemeManager::initialize();

  pvui::MainWindow window;
  window.show();

  return app.exec();
}
