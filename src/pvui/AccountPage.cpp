#include "AccountPage.h"
#include "AutoFillingDelegate.h"
#include "DateUtils.h"
#include "FormatUtils.h"
#include "SecurityPage.h"
#include "TransactionInsertionWidget.h"
#include "pv/Account.h"
#include "pv/Algorithms.h"
#include "pv/DataFile.h"
#include "pv/Integer64.h"
#include "pv/Transaction.h"
#include "pvui/DataFileManager.h"
#include "pvui/ModelUtils.h"
#include <QAbstractProxyModel>
#include <QCheckBox>
#include <QDate>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QTreeView>
#include <cassert>
#include <optional>

pvui::AccountPageWidget::AccountPageWidget(pvui::DataFileManager& dataFileManager, QWidget* parent)
    : PageWidget(parent), dataFileManager(dataFileManager),
      insertWidget(new controls::TransactionInsertionWidget(dataFileManager)) {
  settings.beginGroup("AccountPage");
  // UI Init
  layout()->addWidget(table, 1);
  layout()->addWidget(insertWidget);
  setFocusProxy(insertWidget);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->verticalHeader()->hide();
  table->setSortingEnabled(true);
  table->sortByColumn(0, Qt::AscendingOrder);
  table->setSelectionBehavior(QTableView::SelectRows);
  table->setModel(proxyModel);
  proxyModel->setSortRole(pvui::modelutils::SortRole);
  table->scrollToBottom();
  table->setItemDelegate(new AutoFillingDelegate);

  // Setup delete transaction
  table->setContextMenuPolicy(Qt::ActionsContextMenu);
  deleteTransactionAction.setShortcut(QKeySequence::Delete);
  QObject::connect(&deleteTransactionAction, &QAction::triggered, this, &AccountPageWidget::deleteSelectedTransactions);
  table->addAction(&deleteTransactionAction);

  // Setup listeners
  QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this,
                   &AccountPageWidget::handleDataFileChanged);
  QObject::connect(this, &AccountPageWidget::accountUpdated, this, &AccountPageWidget::handleAccountUpdated);
  QObject::connect(this, &AccountPageWidget::transactionUpdated, this, &AccountPageWidget::handleTransactionsUpdated);
  QObject::connect(this, &AccountPageWidget::reset, this, &AccountPageWidget::handleReset);

  QObject::connect(insertWidget, &controls::TransactionInsertionWidget::submitted, this,
                   &AccountPageWidget::handleTransactionSubmitted);
  QObject::connect(
      proxyModel, &QAbstractProxyModel::sourceModelChanged, this, [this] { table->scrollToBottom(); },
      Qt::ConnectionType::QueuedConnection); // scroll to bottom when the table is changed,
  // For the previous line, we force a queued connection because we need to give it a delay, otherwise scrollToBottom
  // will not work

  handleDataFileChanged(); // Call in constructor to initialize state
}

bool pvui::AccountPageWidget::canDeleteTransactions() {
  if (!account_.has_value() || !dataFileManager.has()) {
    return false;
  }

  if (!settings.value(QStringLiteral("WarnOnTransactionDeletion"), true).toBool()) {
    return true;
  }

  int numberOfTransactionsSelected = table->selectionModel()->selectedRows().size();
  QString accountName = QString::fromStdString(pv::account::name(*dataFileManager, *account_)).toHtmlEscaped();
  QString warningText = tr("<html>Are you sure you want to delete <b>%1</b> transaction(s) from <b>%2</b>?</html>",
                           nullptr, numberOfTransactionsSelected)
                            .arg(numberOfTransactionsSelected)
                            .arg(accountName);

  QMessageBox* warning =
      new QMessageBox(QMessageBox::Question, tr("Delete Transaction(s)?", nullptr, numberOfTransactionsSelected),
                      warningText, QMessageBox::Yes | QMessageBox::Cancel, this);
  QCheckBox* dontShowAgain = new QCheckBox(tr("&Don't show this again"));
  dontShowAgain->setChecked(false);
  warning->setCheckBox(dontShowAgain);
  warning->setAttribute(Qt::WA_DeleteOnClose);
  QObject::connect(warning, &QMessageBox::accepted, this, [dontShowAgain, this]() {
    settings.setValue(QStringLiteral("WarnOnTransactionDeletion"), !dontShowAgain->isChecked());
  });
  return warning->exec() == QMessageBox::Yes;
}

void pvui::AccountPageWidget::deleteSelectedTransactions() {
  if (!canDeleteTransactions()) {
    return;
  }

  std::vector<pv::i64> transactionsToDelete;

  const auto& selectedRows = table->selectionModel()->selectedRows();
  for (const auto& index : selectedRows) {
    transactionsToDelete.push_back(model->transactionOfIndex(index.row()));
  }

  // Use transaction for improved bulk-update performance
  bool sqlTransaction = dataFileManager->beginTransaction() == pv::ResultCode::Ok;
  for (const auto& transaction : transactionsToDelete) {
    dataFileManager->removeTransaction(transaction);
  }
  if (sqlTransaction) {
    dataFileManager->commitTransaction();
  }
}

void pvui::AccountPageWidget::handleDataFileChanged() {
  setAccount(std::nullopt);

  if (dataFileManager.has()) {
    accountUpdatedConnection =
        dataFileManager->onAccountUpdated([this](pv::i64 changedAccount) { emit accountUpdated(changedAccount); });
    transactionAddedConnection =
        dataFileManager->onTransactionAdded([this](pv::i64 transaction) { emit transactionUpdated(transaction); });
    transactionUpdatedConnection =
        dataFileManager->onTransactionUpdated([this](pv::i64 transaction) { emit transactionUpdated(transaction); });
    transactionRemovedConnection = dataFileManager->onTransactionRemoved(
        [this](pv::i64 transaction) { emit transactionUpdated(transaction, true); });
    resetConnection = dataFileManager->onRollback([this] { emit reset(); });
  } else {
    accountUpdatedConnection.disconnect();
    transactionAddedConnection.disconnect();
    transactionUpdatedConnection.disconnect();
    transactionRemovedConnection.disconnect();
  }
}

void pvui::AccountPageWidget::updateCashBalance() noexcept {
  if (!account_) {
    setSubtitle("");
  } else {
    setSubtitle(
        tr("Cash Balance: %1")
            .arg(util::formatMoney(pv::algorithms::cashBalance(*dataFileManager, *account_, currentEpochDate()))));
  }
}

void pvui::AccountPageWidget::handleAccountUpdated(pv::i64 account) {
  if (this->account_ != account) {
    return;
  }
  updateTitle();
}

void pvui::AccountPageWidget::handleTransactionsUpdated(pv::i64 transaction, bool removed) {
  if (removed || pv::transaction::account(*dataFileManager, transaction) == this->account_) {
    // Only updated if the account matches, but if the transaction was removed update anyway
    // since there is no way to tell which account it belonged to
    updateCashBalance();
  }
}

void pvui::AccountPageWidget::handleReset() {
  updateTitle();
  updateCashBalance();
}

void pvui::AccountPageWidget::handleTransactionSubmitted(pv::i64 transaction) {
  assert(dataFileManager.has());
  assert(pv::transaction::account(*dataFileManager, transaction) == account_);
  assert(model != nullptr);

  QModelIndex index = proxyModel->mapFromSource(model->index(model->indexOfTransaction(transaction), 0));
  table->selectRow(index.row());
  table->scrollTo(index);
}

void pvui::AccountPageWidget::updateTitle() {
  setTitle(account_.has_value() ? QString::fromStdString(pv::account::name(*dataFileManager, *account_))
                                : tr("No Account Open"));
}

void pvui::AccountPageWidget::setAccount(std::optional<pv::i64> account) {
  this->account_ = std::move(account);

  setEnabled(this->account_.has_value());

  updateTitle();
  updateCashBalance();

  model = this->account_.has_value() ? std::make_unique<models::TransactionModel>(*dataFileManager, *this->account_)
                                     : nullptr;
  proxyModel->setSourceModel(model.get());
  insertWidget->setAccount(this->account_);
}
