#include "TransactionModel.h"
#include "ActionData.h"
#include "DateUtils.h"
#include "ModelUtils.h"
#include "pv/DataFile.h"
#include "pv/Integer64.h"
#include "pv/Security.h"
#include "pv/Transaction.h"
#include <QAbstractTableModel>
#include <QDate>
#include <QStringLiteral>
#include <Qt>
#include <cassert>
#include <cmath>
#include <optional>
#include <sqlite3.h>

namespace {

constexpr int dateColumn = 0;
constexpr int actionColumn = 1;
constexpr int securityColumn = 2;
constexpr int numberOfSharesColumn = 3;
constexpr int sharePriceColumn = 4;
constexpr int commissionColumn = 5;
constexpr int totalAmountColumn = 6;
constexpr int columnCount = 7;

std::optional<pv::i64> getSecurity(const pv::DataFile& dataFile, pv::i64 transaction) {
  switch (pv::transaction::action(dataFile, transaction)) {
  case pv::Action::BUY:
    return pv::transaction::buySecurity(dataFile, transaction);
  case pv::Action::SELL:
    return pv::transaction::sellSecurity(dataFile, transaction);
  case pv::Action::DEPOSIT:
    return pv::transaction::depositSecurity(dataFile, transaction);
  case pv::Action::WITHDRAW:
    return pv::transaction::withdrawSecurity(dataFile, transaction);
  case pv::Action::DIVIDEND:
    return pv::transaction::dividendSecurity(dataFile, transaction);
  default:
    return std::nullopt;
  }
}

std::optional<pv::i64> getNumberOfShares(const pv::DataFile& dataFile, pv::i64 transaction) {
  switch (pv::transaction::action(dataFile, transaction)) {
  case pv::Action::BUY:
    return pv::transaction::buyNumberOfShares(dataFile, transaction);
  case pv::Action::SELL:
    return pv::transaction::sellNumberOfShares(dataFile, transaction);
  default:
    return std::nullopt;
  }
}

std::optional<pv::i64> getSharePrice(const pv::DataFile& dataFile, pv::i64 transaction) {
  switch (pv::transaction::action(dataFile, transaction)) {
  case pv::Action::BUY:
    return pv::transaction::buySharePrice(dataFile, transaction);
  case pv::Action::SELL:
    return pv::transaction::sellSharePrice(dataFile, transaction);
  default:
    return std::nullopt;
  }
}

std::optional<pv::i64> getCommission(const pv::DataFile& dataFile, pv::i64 transaction) {
  switch (pv::transaction::action(dataFile, transaction)) {
  case pv::Action::BUY:
    return pv::transaction::buyCommission(dataFile, transaction);
  case pv::Action::SELL:
    return pv::transaction::sellCommission(dataFile, transaction);
  default:
    return std::nullopt;
  }
}

std::optional<pv::i64> getTotalAmount(const pv::DataFile& dataFile, pv::i64 transaction) {
  switch (pv::transaction::action(dataFile, transaction)) {
  case pv::Action::BUY:
    return pv::transaction::buyAmount(dataFile, transaction);
  case pv::Action::SELL:
    return pv::transaction::sellAmount(dataFile, transaction);
  case pv::Action::DEPOSIT:
    return pv::transaction::depositAmount(dataFile, transaction);
  case pv::Action::WITHDRAW:
    return pv::transaction::withdrawAmount(dataFile, transaction);
  case pv::Action::DIVIDEND:
    return pv::transaction::dividendAmount(dataFile, transaction);
  default:
    return std::nullopt;
  }
}

bool setNumberOfShares(pv::DataFile& dataFile, pv::i64 transaction, pv::i64 numberOfShares) {
  switch (pv::transaction::action(dataFile, transaction)) {
  case pv::Action::BUY:
    return dataFile.setBuyNumberOfShares(transaction, numberOfShares) == pv::ResultCode::OK;
  case pv::Action::SELL:
    return dataFile.setSellNumberOfShares(transaction, numberOfShares) == pv::ResultCode::OK;
  default:
    return false;
  }
}

bool setSharePrice(pv::DataFile& dataFile, pv::i64 transaction, pv::i64 sharePrice) {
  switch (pv::transaction::action(dataFile, transaction)) {
  case pv::Action::BUY:
    return dataFile.setBuySharePrice(transaction, sharePrice) == pv::ResultCode::OK;
  case pv::Action::SELL:
    return dataFile.setSellSharePrice(transaction, sharePrice) == pv::ResultCode::OK;
  default:
    return false;
  }
}

bool setCommission(pv::DataFile& dataFile, pv::i64 transaction, pv::i64 commission) {
  switch (pv::transaction::action(dataFile, transaction)) {
  case pv::Action::BUY:
    return dataFile.setBuyCommission(transaction, commission) == pv::ResultCode::OK;
  case pv::Action::SELL:
    return dataFile.setSellCommission(transaction, commission) == pv::ResultCode::OK;
  default:
    return false;
  }
}

bool setAmount(pv::DataFile& dataFile, pv::i64 transaction, pv::i64 commission) {
  switch (pv::transaction::action(dataFile, transaction)) {
  case pv::Action::DEPOSIT:
    return dataFile.setDepositAmount(transaction, commission) == pv::ResultCode::OK;
  case pv::Action::WITHDRAW:
    return dataFile.setWithdrawAmount(transaction, commission) == pv::ResultCode::OK;
  case pv::Action::DIVIDEND:
    return dataFile.setDividendAmount(transaction, commission) == pv::ResultCode::OK;
  default:
    return false;
  }
}

} // namespace

pvui::models::TransactionModel::TransactionModel(pv::DataFile& dataFile, pv::i64 account, QObject* parent)
    : QAbstractTableModel(parent), dataFile(dataFile), account(account) {
  repopulate();

  transactionAddedConnection = dataFile.onTransactionAdded([this](pv::i64 transaction) {
    if (pv::transaction::account(this->dataFile, transaction) == this->account) {
      emit transactionAdded(transaction);
    }
  });

  transactionUpdatedConnection = dataFile.onTransactionUpdated([this](pv::i64 transaction) {
    if (pv::transaction::account(this->dataFile, transaction) == this->account) {
      emit transactionUpdated(transaction);
    }
  });

  transactionRemovedConnection =
      dataFile.onTransactionRemoved([this](pv::i64 transaction) { emit transactionRemoved(transaction); });

  resetConnection = dataFile.onRollback([this] { emit reset(); });

  QObject::connect(this, &TransactionModel::transactionAdded, this, &TransactionModel::handleTransactionAdded);
  QObject::connect(this, &TransactionModel::transactionUpdated, this, &TransactionModel::handleTransactionUpdated);
  QObject::connect(this, &TransactionModel::transactionRemoved, this, &TransactionModel::handleTransactionRemoved);
  QObject::connect(this, &TransactionModel::reset, this, &TransactionModel::handleReset);
}

int pvui::models::TransactionModel::indexOfTransaction(pv::i64 transaction) {
  auto iter = std::find(transactions.begin(), transactions.end(), transaction);
  if (iter == transactions.end()) {
    return -1;
  } else {
    return static_cast<int>(iter - transactions.begin());
  }
}

pv::i64 pvui::models::TransactionModel::transactionOfIndex(int rowIndex) {
  if (static_cast<std::size_t>(rowIndex) < transactions.size()) {
    return transactions[rowIndex];
  } else {
    return -1;
  }
}

void pvui::models::TransactionModel::repopulate() {
  transactions.clear();
  auto query = dataFile.query("SELECT Id FROM Transactions WHERE AccountId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(account));
  while (sqlite3_step(&*query) == SQLITE_ROW) {
    transactions.push_back(sqlite3_column_int64(&*query, 0));
  }
}

void pvui::models::TransactionModel::handleTransactionAdded(pv::i64 id) {
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  transactions.push_back(id);
  endInsertRows();
}

void pvui::models::TransactionModel::handleTransactionUpdated(pv::i64 id) {
  int rowIndex = static_cast<int>(std::find(transactions.cbegin(), transactions.cend(), id) - transactions.cbegin());
  emit dataChanged(index(rowIndex, 0), index(rowIndex, columnCount(QModelIndex()) - 1));
}

void pvui::models::TransactionModel::handleTransactionRemoved(pv::i64 id) {
  auto iter = std::find(transactions.cbegin(), transactions.cend(), id);
  int rowIndex = static_cast<int>(iter - transactions.cbegin());
  beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
  transactions.erase(iter);
  endRemoveRows();
}

void pvui::models::TransactionModel::handleReset() {
  beginResetModel();
  repopulate();
  endResetModel();
}

int pvui::models::TransactionModel::columnCount(const QModelIndex&) const { return ::columnCount; }

QVariant pvui::models::TransactionModel::data(const QModelIndex& index, int role) const {
  if (role == Qt::TextAlignmentRole) {
    auto col = index.column();
    if (col == dateColumn || col == actionColumn || col == securityColumn) {
      return QVariant(); // Use default alignment for non-numeric column
    } else {
      return QVariant(Qt::AlignRight | Qt::AlignVCenter); // Right align
    }
  }
  if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::AccessibleTextRole && role != modelutils::SortRole) {
    return QVariant();
  }
  pv::i64 transaction = transactions.at(index.row());

  switch (index.column()) {
  case dateColumn:
    return toQDate(pv::transaction::date(dataFile, transaction));
  case actionColumn: {
    auto* actionData = pvui::actionData(pv::transaction::action(dataFile, transaction));
    return actionData != nullptr ? actionData->name : QVariant();
  }
  case securityColumn: {
    std::optional<pv::i64> securityId = getSecurity(dataFile, transaction);
    return modelutils::stringData(securityId ? QString::fromStdString(pv::security::symbol(dataFile, *securityId)) : QString(), role);
  }
  case numberOfSharesColumn: {
    std::optional<pv::i64> numberOfShares = getNumberOfShares(dataFile, transaction);
    return numberOfShares ? modelutils::numberData(*numberOfShares, role) : (role == modelutils::SortRole ? modelutils::lowestData() : QVariant());
  }
  case sharePriceColumn: {
    std::optional<pv::i64> sharePrice = getSharePrice(dataFile, transaction);
    return sharePrice ? modelutils::moneyData(*sharePrice, role) : (role == modelutils::SortRole ? modelutils::lowestData() : QVariant());
  }
  case commissionColumn: {
    std::optional<pv::i64> commission = getCommission(dataFile, transaction);
    return commission ? modelutils::moneyData(*commission, role) : (role == modelutils::SortRole ? modelutils::lowestData() : QVariant());
  }
  case totalAmountColumn: {
    std::optional<pv::i64> totalAmount = getTotalAmount(dataFile, transaction);
    return totalAmount ? modelutils::moneyData(*totalAmount, role) : (role == modelutils::SortRole ? modelutils::lowestData() : QVariant());
  }
  default:
    return QVariant();
  }
}

Qt::ItemFlags pvui::models::TransactionModel::flags(const QModelIndex& index) const {
  static const Qt::ItemFlags NonEditable = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  static const Qt::ItemFlags Editable = NonEditable | Qt::ItemIsEditable;

  switch (pv::transaction::action(dataFile, transactions.at(index.row()))) {
  case pv::Action::BUY: // Fall through
  case pv::Action::SELL: {
    if (index.column() == numberOfSharesColumn || index.column() == sharePriceColumn ||
        index.column() == commissionColumn) {
      return Editable;
    } else {
      return NonEditable;
    }
  }
  case pv::Action::DEPOSIT:  //  Fall through
  case pv::Action::WITHDRAW: // Fall through
  case pv::Action::DIVIDEND: {
    if (index.column() == totalAmountColumn) {

      return Editable;
    } else {
      return NonEditable;
    }
  }
  default:
    return NonEditable;
  }
}

bool pvui::models::TransactionModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role != Qt::EditRole)
    return false;

  if (!value.canConvert<double>())
    return false;

  pv::i64 transaction = transactionOfIndex(index.row());

  switch (index.column()) {
  case numberOfSharesColumn:
    return setNumberOfShares(dataFile, transaction, value.toLongLong());
  case sharePriceColumn:
    return setSharePrice(dataFile, transaction, std::llround(value.toDouble() * 100));
  case commissionColumn:
    return setCommission(dataFile, transaction, std::llround(value.toDouble() * 100));
  case totalAmountColumn:
    return setAmount(dataFile, transaction, std::llround(value.toDouble() * 100));
  default:
    return false;
  }
}

QVariant pvui::models::TransactionModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Orientation::Vertical || (role != Qt::DisplayRole && role != Qt::AccessibleTextRole))
    return QVariant();
  switch (section) {
  case 0:
    return tr("Date");
  case 1:
    return tr("Action");
  case 2:
    return tr("Security");
  case 3:
    return tr("# Of Shares");
  case 4:
    return tr("Share Price");
  case 5:
    return tr("Commission");
  case 6:
    return tr("Total Amount");
  default:
    return QVariant();
  }
}
