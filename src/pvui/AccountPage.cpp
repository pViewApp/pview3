#include "AccountPage.h"
#include "SecurityPage.h"
#include "TransactionInsertionWidget.h"
#include "pv/Algorithms.h"
#include <QDate>
#include <QHeaderView>
#include <QLineEdit>
#include <QTreeView>
#include <algorithm>
#include <cassert>
#include <date/date.h>
#include <fmt/format.h>
#include <string>

void pvui::AccountPageWidget::updateCashBalance() noexcept {
  if (account_ == nullptr) {
    setSubtitle("");
  } else {
    setSubtitle(tr("Cash Balance: %1").arg("$" + QString::fromStdString(pv::algorithms::cashBalance(*account_).str())));
  }
}

void pvui::AccountPageWidget::updateTitle() {
  setTitle(account_ != nullptr ? QString::fromStdString(account_->name()) : tr("No Account Open"));
}

pvui::AccountPageWidget::AccountPageWidget(pv::DataFile* dataFile, pv::Account* account, QWidget* parent)
    : PageWidget(parent) {
  layout()->addWidget(table, 1);
  layout()->addWidget(insertWidget);

  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->verticalHeader()->hide();

  table->setSortingEnabled(true);
  table->sortByColumn(0, Qt::AscendingOrder);

  table->setSelectionBehavior(QTableView::SelectRows);

  table->setModel(proxyModel);
  table->scrollToBottom();

  // Setup delete transaction
  deleteTransactionAction.setShortcut(QKeySequence::Delete);
  QObject::connect(&deleteTransactionAction, &QAction::triggered, this, [&]() {
    std::vector<std::size_t> transactionsToDelete;
    transactionsToDelete.reserve(table->selectionModel()->selectedRows().length());
    for (const auto& index : table->selectionModel()->selectedRows()) {

      assert(account_->transactions().size() > static_cast<std::size_t>(index.row()) &&
             "Model row doesn't match with transaction indexes.");
      transactionsToDelete.push_back(index.row());
    }

    for (const auto& transaction : transactionsToDelete) {
      account_->removeTransaction(transaction);
    }
  });

  table->addAction(&deleteTransactionAction);
  table->setContextMenuPolicy(Qt::ActionsContextMenu);

  // Setup listeners
  QObject::connect(this, &AccountPageWidget::accountTransactionsChanged, this, &AccountPageWidget::updateCashBalance);
  QObject::connect(this, &AccountPageWidget::accountNameChanged, this, &AccountPageWidget::updateTitle);

  setAccount(dataFile, account);
}

void pvui::AccountPageWidget::setAccount(pv::DataFile* dataFile, pv::Account* account) {
  account_ = account;

  setEnabled(account_ != nullptr);
  transactionAddedConnection.disconnect();
  transactionRemovedConnection.disconnect();
  transactionChangedConnection.disconnect();

  updateTitle();

  model = account_ != nullptr ? std::make_unique<models::TransactionModel>(*dataFile, *account_) : nullptr;
  proxyModel->setSourceModel(model.get());
  insertWidget->setAccount(dataFile, account_);

  if (!isEnabled())
    return;
  updateCashBalance();
  setTitle(QString::fromStdString(account_->name()));
  transactionAddedConnection =
      account_->listenTransactionAdded([&](std::size_t, const pv::Transaction*) { emit accountTransactionsChanged(); });
  transactionRemovedConnection =
      account_->listenTransactionRemoved([&](std::size_t) { emit accountTransactionsChanged(); });
  transactionChangedConnection = account_->listenTransactionReplaced(
      [&](std::size_t, const pv::Transaction*, const pv::Transaction*) { emit accountTransactionsChanged(); });

  table->scrollToBottom();

  accountNameChangedConnection =
      account->listenNameChanged([&](std::string, std::string) { emit accountNameChanged(); });
}
