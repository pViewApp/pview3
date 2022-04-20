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
#include "DataFile.h"
#include "DataFileManager.h"
#include "SecurityPage.h"

namespace pvui {
class MainWindow : public QMainWindow {
  Q_OBJECT
private:
  QTreeView* navigationWidget;
  QStandardItemModel* navigationModel;
  QWidget* content;
  QLabel* noPageOpen;
  QStackedLayout* contentLayout;
  AccountPageWidget* accountPage;
  DataFileManager dataFileManager;
  SecurityPageWidget* securityPage = new SecurityPageWidget(dataFileManager);
  QStandardItem* m_navigationAccountItem;

  QStandardItem* securitiesNavigationItem = new QStandardItem(tr("Securities"));
  std::unordered_map<QStandardItem*, pv::AccountPtr> accountNavigationItems;

  void setupDataFile();

public:
  MainWindow(QWidget* parent = nullptr);

  void setupMenuBar();
  void setupNavigation();

protected:
  inline pv::DataFile& dataFile() noexcept { return dataFileManager.dataFile(); }
protected slots:
  void pageChanged(const QItemSelection& selection);
  void addAccount();
};
} // namespace pvui
#endif // PVUI_MAINWINDOW_H
