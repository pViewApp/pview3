#include "TransactionModel.h"
#include "ActionMappings.h"
#include "pv/Action.h"
#include <QDate>

pvui::models::TransactionModel::TransactionModel(const pv::Account account, QObject* parent)
    : QAbstractTableModel(parent), account_(account), transactions(account.transactions()) {
  QObject::connect(this, &TransactionModel::transactionAdded, this, [&](pv::Transaction& transaction) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    transactions.push_back(transaction);
    endInsertRows();
  });

  transactionAddedConnection =
      account.transactionAdded().connect([&](pv::Transaction transaction) { emit transactionAdded(transaction); });
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
