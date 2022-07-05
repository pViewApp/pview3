#ifndef PVUI_MAINWINDOW_H
#define PVUI_MAINWINDOW_H

#include <QEvent>
#include <QItemDelegate>
#include <QLabel>
#include <QSettings>
#include <QMainWindow>
#include <QSplitter>
#include <QStackedLayout>
#include <QStandardItemModel>
#include <QToolBar>
#include <QTreeView>
#include <QWidget>
#include <unordered_map>
#include <QMenu>

#include "AccountPage.h"
#include "DataFileManager.h"
#include "NavigationModel.h"
#include "SecurityPage.h"
#include "StandardReportFactory.h"
#include "pv/DataFile.h"

namespace pvui {
class MainWindow : public QMainWindow {
  Q_OBJECT
private:
  QSettings settings;
  QToolBar* mainToolBar = new QToolBar(this);
  QToolBar* currentToolBar = nullptr;
  DataFileManager dataFileManager;
  QTreeView* navigationWidget = new QTreeView;

  StandardReportFactory factory;
  std::vector<Report*> reports = factory.createReports(dataFileManager);
  models::NavigationModel navigationModel = models::NavigationModel(dataFileManager);
  QWidget* content = new QWidget;
  QLabel* noPageOpen = new QLabel(tr("No Page Open"), content);

  QStackedLayout* contentLayout = new QStackedLayout(content);
  AccountPageWidget* accountPage = new AccountPageWidget(dataFileManager);
  SecurityPageWidget* securityPage = new SecurityPageWidget(dataFileManager);

  //// MENU BAR
  
  QMenu fileMenu;
  QAction fileNewAction;
  QAction fileOpenAction;
  QAction fileQuitAction;

  QMenu accountsMenu;
  QAction accountsNewAction;
  QAction accountsDeleteAction;

  void setupToolBars();
  void hideToolBars();
private slots:
  void handleDataFileChanged();
  void pageChanged();

  void updateTitle();

  // Action handlers
  void fileNew();
  void fileOpen();
  void fileQuit();

  void accountsNew();
  void accountsDelete();

  void fileOpen_(std::string location); // Actual implementation of file-opening logic
protected:
  void closeEvent(QCloseEvent* event) override;
public:
  MainWindow(QWidget* parent = nullptr);

  void setupToolBar(QToolBar* toolBar);
  void setupMenuBar();
  void setupNavigation();
};
} // namespace pvui
#endif // PVUI_MAINWINDOW_H
