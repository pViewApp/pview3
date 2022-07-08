#ifndef PVUI_ACCOUNTPAGE_H
#define PVUI_ACCOUNTPAGE_H

#include "ExtendedSpinBox.h"
#include "Page.h"
#include "TransactionInsertionWidget.h"
#include "TransactionModel.h"
#include "pv/Account.h"
#include "pv/Integer64.h"
#include "pv/Signals.h"
#include "pvui/DataFileManager.h"
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QWidget>
#include <QSettings>
#include <QAction>

namespace pvui {

class AccountPageWidget : public PageWidget {
  Q_OBJECT
private:
  DataFileManager& dataFileManager;
  std::optional<pv::i64> account_;

  pv::ScopedConnection accountUpdatedConnection;
  pv::ScopedConnection transactionAddedConnection;
  pv::ScopedConnection transactionRemovedConnection;
  pv::ScopedConnection transactionUpdatedConnection;
  pv::ScopedConnection resetConnection;

  QSettings settings;
  QTableView* table = new QTableView;
  controls::TransactionInsertionWidget* insertWidget;
  QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(table);
  std::unique_ptr<models::TransactionModel> model = nullptr;

  QAction deleteTransactionAction = QAction(tr("Delete Selected Transactions"));

  void updateCashBalance() noexcept;
  void updateTitle();
private slots:
  void handleAccountUpdated(pv::i64 account);
  void handleTransactionsUpdated(pv::i64 transaction, bool removed);
  void handleReset();

  void handleTransactionSubmitted(pv::i64 transaction);

  bool canDeleteTransactions();
  void deleteSelectedTransactions();

  void handleDataFileChanged();
public:
  AccountPageWidget(DataFileManager& dataFileManager, QWidget* parent = nullptr);
public slots:
  void setAccount(std::optional<pv::i64> account);
signals:
  void accountUpdated(pv::i64 account);
  void transactionUpdated(pv::i64 transaction, bool removed = false);
  void reset();
};

} // namespace pvui

#endif // PVUI_ACCOUNT_PAGE_H
