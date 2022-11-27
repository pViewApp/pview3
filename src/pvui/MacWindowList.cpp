#include "MacWindowList.h"

namespace pvui {
namespace mac {

WindowList::WindowList() : windowActionsGroup(this), menu(tr("Window")) {
  zoomAction.setText(tr("Zoom"));
  minimizeAction.setText(tr("Minimize"));
}

void WindowList::repopulateWindowList() { menu.clear();
  menu.addAction(&minimizeAction);
  menu.addAction(&zoomAction);
  menu.addSeparator();
  for (auto* windowAction : windowActions) {
    menu.addAction(windowAction);
  }
}

void WindowList::addWindow(QWidget* window) {
  windows.push_back(window);
  QAction* windowAction = new QAction(window->windowTitle(), &windowActionsGroup);
  windowAction->setCheckable(true);
  windowActions << windowAction;
  QObject::connect(window, &QWidget::windowTitleChanged, this, &WindowList::onWindowTitleChanged);
  QObject::connect(window, &QWidget::destroyed, this, [this, window]() { removeWindow(window); });
  QObject::connect(windowAction, &QAction::toggled, this, [=] { window->activateWindow(); window->raise(); });
  repopulateWindowList();
}

void WindowList::removeWindow(QWidget* window) {
  auto iter = std::find(windows.begin(), windows.end(), window);
  if (iter == windows.end()) return;
  qsizetype index = iter - windows.begin();
  windows.remove(index);
  menu.clear();
  windowActions.remove(index);
  repopulateWindowList();
}

void WindowList::setActiveWindow(QWidget* window) {
  qsizetype index = std::find(windows.begin(), windows.end(), window) - windows.begin();
  if (windowActionsGroup.checkedAction()) {
    windowActionsGroup.checkedAction()->setChecked(false);
  }
  windowActions.at(index)->setChecked(true);
}

void WindowList::onWindowTitleChanged() {
  for (qsizetype i = 0; i < windows.length(); ++i) {
    windowActions.at(i)->setText(windows.at(i)->windowTitle());
  }
}

}
}