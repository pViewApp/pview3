#ifndef PVUI_MAINWINDOW_H
#define PVUI_MAINWINDOW_H

#include <QEvent>
#include <QItemDelegate>
#include <QLabel>
#include <QMainWindow>
#include <QSplitter>
#include <QStackedLayout>
#include <QStandardItemModel>
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

  // navigationWidget context menu actions
  QAction deleteAccountAction = QAction(tr("&Delete Account"));
  QAction newAccountAction = QAction(tr("New Account..."));
private slots:
  void pageChanged();

public:
  MainWindow(QWidget* parent = nullptr);

  void setupMenuBar();
  void setupNavigation();

protected:
  pv::DataFile& dataFile() noexcept { return dataFileManager.dataFile(); }
protected slots:
  // Dialogs
  void showDeleteAccountDialog();
  void showAddAccountDialog();
};
} // namespace pvui
#endif // PVUI_MAINWINDOW_H
