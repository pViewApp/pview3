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
#include "SettingsDialog.h"
#include "AccountPage.h"
#include "DataFileManager.h"
#include "NavigationModel.h"
#include "SecurityPage.h"
#include "StandardReportFactory.h"
#include "pv/DataFile.h"

class QDropEvent;
class QDragEnterEvent;

namespace pvui {
class MainWindow : public QMainWindow {
  Q_OBJECT
private:
  QSettings settings;
  DataFileManager dataFileManager;
  QTreeView* navigationWidget = new QTreeView;
  QSplitter splitter;

  QLabel* statusBarLabel = new QLabel;

  StandardReportFactory factory;
  std::vector<Report*> reports = factory.createReports(dataFileManager);
  models::NavigationModel navigationModel = models::NavigationModel(dataFileManager);
  QWidget* content = new QWidget;
  QLabel* noPageOpen = new QLabel(tr("No Page Open"), content);

  QStackedLayout* contentLayout = new QStackedLayout(content);
  AccountPageWidget* accountPage = new AccountPageWidget(dataFileManager);
  SecurityPageWidget* securityPage = new SecurityPageWidget(dataFileManager);

  dialogs::SettingsDialog settingsDialog;

  //// MENU BAR
  
  QMenu fileMenu;
  QAction fileNewAction;
  QAction fileOpenAction;
  QAction fileSettingsAction;
  QAction fileQuitAction;

  QMenu accountsMenu;
  QAction accountsNewAction;
  QAction accountsDeleteAction;

  QMenu helpMenu;
  QAction helpAboutAction;
private slots:
  void handleDataFileChanged();
  void pageChanged();

  void updateWindowFileLocation();

  // Action handlers
  void fileNew();
  void fileOpen();
  void fileSettings();
  void fileQuit();

  void accountsNew();
  void accountsDelete();

  void helpAbout();

  void fileOpen_(const std::string&
                     location); // Actual implementation of file-opening logic (opens a file, throws exception if fail)
  void fileOpenWithWarning_(const std::string& location) noexcept; // Opens a file, warns the user if failed

  void setupActions();
  void setupNavigation();

protected:
  void closeEvent(QCloseEvent* event) override;
public:
  MainWindow(QWidget* parent = nullptr);

  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  bool nativeEvent(const QByteArray& eventType, void* message, long* result) override;
};
} // namespace pvui
#endif // PVUI_MAINWINDOW_H
