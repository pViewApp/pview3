#ifndef PVUI_MAINWINDOW_H
#define PVUI_MAINWINDOW_H

#include <QEvent>
#include <QItemDelegate>
#include <QLabel>
#include <QMainWindow>
#include <QSplitter>
#include <QStackedLayout>
#include <QStandardItemModel>
#include <QToolBar>
#include <QTreeView>
#include <QWidget>
#include <unordered_map>

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
  AccountPageWidget* accountPage = new AccountPageWidget;
  SecurityPageWidget* securityPage = new SecurityPageWidget(dataFileManager);

  // General actions
  QAction newAccountAction = QAction(tr("New Account..."));
  // navigationWidget context menu actions
  QAction deleteAccountAction = QAction(tr("&Delete Account"));

  void setupToolBars();
  void hideToolBars();
private slots:
  void pageChanged();

protected:
  pv::DataFile& dataFile() noexcept { return dataFileManager.dataFile(); }
  void closeEvent(QCloseEvent* event) override;
protected slots:
  // Dialogs
  void showDeleteAccountDialog();
  void showAddAccountDialog();

public:
  MainWindow(QWidget* parent = nullptr);

  void setupToolBar(QToolBar* toolBar);
  void setupMenuBar();
  void setupNavigation();
};
} // namespace pvui
#endif // PVUI_MAINWINDOW_H
