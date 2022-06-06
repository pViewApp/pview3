#ifndef PVUI_ACCOUNTPAGE_H
#define PVUI_ACCOUNTPAGE_H

#include "ExtendedSpinBox.h"
#include "Page.h"
#include "TransactionInsertionWidget.h"
#include "TransactionModel.h"
#include "pv/Account.h"
#include <QAction>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QShowEvent>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVariant>
#include <QWidget>
#include <boost/signals2.hpp>
#include <boost/signals2/connection.hpp>
#include <memory>
#include <optional>

namespace pvui {

class AccountPageWidget : public PageWidget {
  Q_OBJECT
private:
  pv::Account* account_ = nullptr;
  QTableView* table = new QTableView;
  controls::TransactionInsertionWidget* insertWidget = new controls::TransactionInsertionWidget();
  QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(table);
  std::unique_ptr<models::TransactionModel> model = nullptr;

  boost::signals2::scoped_connection accountNameChangedConnection;
  boost::signals2::scoped_connection transactionAddedConnection;
  boost::signals2::scoped_connection transactionRemovedConnection;
  boost::signals2::scoped_connection transactionChangedConnection;

  QAction deleteTransactionAction = QAction(tr("Delete Selected Transactions"));

public:
  AccountPageWidget(pv::DataFile* dataFile = nullptr, pv::Account* account = nullptr, QWidget* parent = nullptr);
private slots:
  void updateCashBalance() noexcept;
  void updateTitle();
public slots:
  void setAccount(pv::DataFile* dataFile, pv::Account* account);
signals:
  void accountTransactionsChanged();
  void accountNameChanged();
};

} // namespace pvui

#endif // PVUI_ACCOUNT_PAGE_H
