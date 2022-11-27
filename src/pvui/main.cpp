#include "MainWindow.h"
#include "ThemeManager.h"
#include <QtGlobal>
#include "MacWindowList.h"
#include <QApplication>
#include <QCoreApplication>
#include <QIcon>

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

  QCoreApplication::setOrganizationName("pView");
  QCoreApplication::setOrganizationDomain("pviewapp.github.io");
  QCoreApplication::setApplicationName("pView");

  installFallbackIcons();
  QApplication app(argc, argv);

  pvui::ThemeManager::initialize();

  pvui::mac::WindowList windowList;
  pvui::MainWindow* window = new pvui::MainWindow(windowList);
  window->show();
  window->setAttribute(Qt::WA_DeleteOnClose);

  return app.exec();
}
