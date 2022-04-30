#include "AccountPage.h"
#include "SecurityPage.h"
#include "TransactionInsertionWidget.h"
#include "pv/Actions.h"
#include "pv/Algorithms.h"
#include <QDate>
#include <QHeaderView>
#include <QLineEdit>
#include <QTreeView>
#include <algorithm>
#include <date/date.h>
#include <fmt/format.h>
#include <string>

void pvui::AccountPageWidget::updateCashBalance() noexcept {
  if (!account_.has_value()) {
    setSubtitle("");
    return;
  }

  setSubtitle(tr("Cash Balance: %1").arg("$" + QString::fromStdString(pv::algorithms::cashBalance(*account_).str())));
}

void pvui::AccountPageWidget::updateTitle() {
  setTitle(account_.has_value() ? QString::fromStdString(account_->name()) : tr("No Account Open"));
}

pvui::AccountPageWidget::AccountPageWidget(std::optional<pv::Account> account, QWidget* parent) : PageWidget(parent) {
  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(table, 1);
  layout->addWidget(insertWidget);
  setContent(layout);

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
    std::vector<pv::Transaction> transactionsToDelete;
    transactionsToDelete.reserve(table->selectionModel()->selectedRows().length());
    for (const auto& index : table->selectionModel()->selectedRows()) {
      std::optional<pv::Transaction> transaction = model->mapFromIndex(index);
      if (transaction.has_value()) {
        transactionsToDelete.push_back(*transaction);
      }
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

  setAccount(account);
}

void pvui::AccountPageWidget::setAccount(std::optional<pv::Account> account) {
  account_ = account;

  setEnabled(account_.has_value());
  transactionAddedConnection.disconnect();
  transactionRemovedConnection.disconnect();
  transactionChangedConnection.disconnect();

  updateTitle();

  model = account_.has_value() ? std::make_unique<models::TransactionModel>(*account_) : nullptr;
  proxyModel->setSourceModel(model.get());
  insertWidget->setAccount(account_);

  if (!isEnabled())
    return;
  updateCashBalance();
  setTitle(QString::fromStdString(account_->name()));
  transactionAddedConnection =
      account_->transactionAdded().connect([&](const pv::Transaction&) { emit accountTransactionsChanged(); });
  transactionRemovedConnection =
      account_->transactionRemoved().connect([&](const pv::Transaction&) { emit accountTransactionsChanged(); });
  transactionChangedConnection =
      account_->transactionChanged().connect([&](const pv::Transaction&) { emit accountTransactionsChanged(); });

  table->scrollToBottom();

  accountNameChangedConnection =
      account->nameChanged().connect([&](std::string newName, std::string) { emit accountNameChanged(); });
}
