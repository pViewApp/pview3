#include "TransactionModel.h"
#include "ActionMappings.h"
#include "pv/Action.h"
#include <QDate>
#include <cassert>

pvui::models::TransactionModel::TransactionModel(const pv::Account account, QObject* parent)
    : QAbstractTableModel(parent), account_(account), transactions(account.transactions()) {

  // Listen for added transactions
  QObject::connect(this, &TransactionModel::transactionAdded, this, [&](const pv::Transaction& transaction) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    transactions.push_back(transaction);
    endInsertRows();
  });

  transactionAddedConnection =
      account.transactionAdded().connect([&](pv::Transaction transaction) { emit transactionAdded(transaction); });

  // Listen for changed transactions
  QObject::connect(this, &TransactionModel::transactionChanged, this, [&](const pv::Transaction& transaction) {
    int rowIndex = std::find(transactions.cbegin(), transactions.cend(), transaction) - transactions.cbegin();

    QModelIndex topLeft = index(rowIndex, 0);
    QModelIndex bottomRight = index(rowIndex, columnCount(QModelIndex()));

    emit dataChanged(topLeft, bottomRight);
  });

  transactionChangedConnection = account.transactionChanged().connect(
      [&](const pv::Transaction& transaction) { emit transactionChanged(transaction); });

  // Listen for removed transactions
  QObject::connect(this, &TransactionModel::transactionRemoved, this, [&](const pv::Transaction& transaction) {
    auto iter = std::find(transactions.cbegin(), transactions.cend(), transaction);
    assert(iter != transactions.cend() && "Transaction removed but not in TransactionModel");
    int rowIndex = iter - transactions.cbegin();
    beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
    transactions.erase(iter);
    endRemoveRows();
  });

  transactionRemovedConnection =
      account.transactionRemoved().connect([&](pv::Transaction transaction) { emit transactionRemoved(transaction); });
}

QVariant pvui::models::TransactionModel::data(const QModelIndex& index, int role) const {
  if ((role != Qt::EditRole && role != Qt::DisplayRole) || !index.isValid())
    return QVariant();

  const pv::Transaction& transaction = transactions.at(index.row());
  switch (index.column()) {
  case 0: {
    date::year_month_day ymd{transaction.date()};
    return QDate(static_cast<int>(ymd.year()), static_cast<unsigned int>(ymd.month()),
                 static_cast<unsigned int>(ymd.day()));
  }
  case 1: {
    using namespace actionmappings;
    auto iter = actionToNameMappings.find(&transaction.action());
    if (iter == actionToNameMappings.cend()) {
      return QString::fromStdString(transaction.action().id());
    } else {
      return tr(iter->second.c_str());
    }
  }
  case 2: {
    return transaction.security().has_value() ? QString::fromStdString(transaction.security()->symbol()) : "";
  }
  case 3: {
    return QString::fromStdString(transaction.numberOfShares().str());
  }
  case 4: {
    return QString::fromStdString(transaction.sharePrice().str());
  }
  case 5: {
    return QString::fromStdString(transaction.commission().str());
  }
  case 6: {
    return QString::fromStdString(transaction.totalAmount().str());
  }
  default: {
    return QVariant();
  }
  }
}

Qt::ItemFlags pvui::models::TransactionModel::flags(const QModelIndex& index) const {
  static const Qt::ItemFlags NonEditable = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  static const Qt::ItemFlags Editable = NonEditable | Qt::ItemIsEditable;

  if (index.column() >= 3) {
    return Editable;
  } else {
    return NonEditable;
  }
}

bool pvui::models::TransactionModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role != Qt::EditRole)
    return false;

  auto transaction = transactions.at(index.row());
  if (!value.canConvert<double>())
    return false;

  double newValue = value.value<double>();
  switch (index.column()) {
  case 3: {
    return transaction.setNumberOfShares(newValue) == pv::TransactionEditResult::Success;
  }
  case 4: {
    return transaction.setSharePrice(newValue) == pv::TransactionEditResult::Success;
  }
  case 5: {
    return transaction.setCommission(newValue) == pv::TransactionEditResult::Success;
  }
  case 6: {
    return transaction.setTotalAmount(newValue) == pv::TransactionEditResult::Success;
  }
  }
  return false;
}

QModelIndex pvui::models::TransactionModel::mapToIndex(const pv::Transaction& transaction) const noexcept {
  auto iter = std::find(transactions.cbegin(), transactions.cend(), transaction);
  if (iter == transactions.cend()) {
    return QModelIndex();
  }
  return index(iter - transactions.begin(), 0);
}

std::optional<pv::Transaction> pvui::models::TransactionModel::mapFromIndex(const QModelIndex& index) noexcept {
  if (!index.isValid() || index.row() >= rowCount())
    return std::nullopt;
  return transactions.at(index.row());
}

QVariant pvui::models::TransactionModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Orientation::Vertical || role != Qt::DisplayRole)
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
