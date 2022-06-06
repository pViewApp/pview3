#ifndef PVUI_SECURITY_PAGE_H
#define PVUI_SECURITY_PAGE_H

#include "DataFileManager.h"
#include "Page.h"
#include "SecurityInsertionWidget.h"
#include "SecurityModel.h"
#include "pv/Security.h"
#include <QAction>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QRegularExpression>
#include <QShowEvent>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QToolBar>
#include <QValidator>
#include <memory>
#include <optional>

namespace pvui {

class SecurityPageWidget : public PageWidget {
  Q_OBJECT
private:
  pvui::DataFileManager& dataFileManager_;

  QLabel* toolBarTitleLabel = new QLabel();
  QToolBar* toolBar_ = new QToolBar(this);
  QTableView* table = new QTableView;
  controls::SecurityInsertionWidget* insertionWidget = new controls::SecurityInsertionWidget(dataFileManager_);

  QSortFilterProxyModel proxyModel = QSortFilterProxyModel(table);
  std::unique_ptr<models::SecurityModel> model = nullptr;

  QAction securityInfoAction = QAction(tr("Edit Security Prices..."));
  QAction deleteSecurityAction = QAction(tr("Delete Security"));

  void setupActions();
  void setDataFile(pv::DataFile& dataFile);

  pv::Security* currentSelectedSecurity();

  void setToolBarLabel(pv::Security* security);

public:
  SecurityPageWidget(pvui::DataFileManager& dataFileManager, QWidget* parent = nullptr);

  QToolBar* toolBar() override { return toolBar_; }
};

} // namespace pvui

#endif // PVUI_SECURITY_PAGE_H
