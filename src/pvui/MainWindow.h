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
#include <QProgressBar>
#include <unordered_map>
#include <QMenu>
#include "SettingsDialog.h"
#include "AccountPage.h"
#include "DataFileManager.h"
#include "NavigationModel.h"
#include "SecurityPage.h"
#include "StandardReportFactory.h"
#include "pv/DataFile.h"
#include "MacWindowList.h"

class QDropEvent;
class QDragEnterEvent;

namespace pvui {
class MainWindow : public QMainWindow {
  Q_OBJECT
private:
  mac::WindowList& windowList;

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
  QProgressBar securityPriceDownloadProgressBar;

  dialogs::SettingsDialog settingsDialog;

  //// MENU BAR
  
  QMenu fileMenu;
  QAction fileNewAction;
  QAction fileOpenAction;
  QAction fileSettingsAction;
  QAction fileNewWindowAction;
#ifdef Q_OS_MACOS
  QAction fileCloseWindowAction;
#endif
  QAction fileQuitAction;

  QMenu accountsMenu;
  QAction accountsNewAction;
  QAction accountsDeleteAction;

  QMenu helpMenu;
  QAction helpAboutAction;
  QAction helpWebsiteAction;
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
  void helpWebsite();

  void fileOpen_(const std::string&
                     location); // Actual implementation of file-opening logic (opens a file, throws exception if fail)
  void fileOpenWithWarning_(const std::string& location) noexcept; // Opens a file, warns the user if failed

  void fileNewWindow();
  void setupActions();
  void setupNavigation();

protected:
  void closeEvent(QCloseEvent* event) override;
public:
  MainWindow(mac::WindowList& windowList, QWidget* parent = nullptr);

  bool event(QEvent* e) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  virtual bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
};
} // namespace pvui
#endif // PVUI_MAINWINDOW_H
