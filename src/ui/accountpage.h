#ifndef PVUI_ACCOUNTPAGE_H
#define PVUI_ACCOUNTPAGE_H

#include "DataFile.h"
#include "ExtendedSpinBox.h"
#include "Page.h"
#include "TransactionInsertionWidget.h"
#include "TransactionModel.h"
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QShowEvent>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVariant>
#include <QWidget>
#include <boost/signals2.hpp>
#include <memory>
#include <optional>

namespace pvui {

class AccountPageWidget : public PageWidget {
  Q_OBJECT
private:
  pv::AccountPtr account_ = nullptr;
  QTableView* table = new QTableView;
  controls::TransactionInsertionWidget* insertWidget = new controls::TransactionInsertionWidget;
  QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(table);
  std::unique_ptr<models::TransactionModel> model = nullptr;

public:
  AccountPageWidget(pv::AccountPtr account = nullptr, QWidget* parent = nullptr);
public slots:
  void setAccount(pv::AccountPtr account);
};

} // namespace pvui

#endif // PVUI_ACCOUNT_PAGE_H
