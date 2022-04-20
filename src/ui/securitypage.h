#ifndef PVUI_SECURITY_PAGE_H
#define PVUI_SECURITY_PAGE_H

#include "DataFileManager.h"
#include "Page.h"
#include "SecurityInsertionWidget.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QRegularExpression>
#include <QShowEvent>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QToolBar>
#include <QValidator>

namespace pvui {

class SecurityPageWidget : public PageWidget {
  Q_OBJECT
private:
  pvui::DataFileManager& dataFileManager_;

  QToolBar* toolBar = new QToolBar;
  QTableView* table = new QTableView;
  controls::SecurityInsertionWidget* insertionWidget = new controls::SecurityInsertionWidget(dataFileManager_);

  QAction* securityInfoAction = new QAction(tr("Edit Security Prices..."));

  QSortFilterProxyModel* tableModel = new QSortFilterProxyModel;

  void setupToolbar();

public:
  SecurityPageWidget(pvui::DataFileManager& dataFileManager, QWidget* parent = nullptr);
};
} // namespace pvui

#endif // PVUI_SECURITY_PAGE_H
