#ifndef PVUI_MAC_WINDOWLIST_H
#define PVUI_MAC_WINDOWLIST_H
#include <QList>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QActionGroup>

namespace pvui {
namespace mac {

class WindowList : public QObject {
private:
  QList<QWidget*> windows;
  QList<QAction*> windowActions;
  QAction minimizeAction;
  QAction zoomAction;
  QActionGroup windowActionsGroup;
  QMenu menu;
public:
  WindowList();
  // Do not modify the menu pls!
  QMenu* windowMenu() {
    return &menu;
  }
public slots:
  void addWindow(QWidget* window);
  void removeWindow(QWidget* window);
  void setActiveWindow(QWidget* window);
private slots:
  void onWindowTitleChanged();
  // Call every time a window is added or removed
  void repopulateWindowList();
};

}
}
#endif // PVUI_MAC_WINDOWLIST_H