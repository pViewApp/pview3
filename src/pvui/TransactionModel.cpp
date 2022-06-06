#include "TransactionModel.h"
#include "ActionData.h"
#include "ModelUtils.h"
#include "pv/DataFile.h"
#include <QDate>
#include <cassert>

namespace {

constexpr int dateColumn = 0;
constexpr int actionColumn = 1;
constexpr int securityColumn = 2;
constexpr int numberOfSharesColumn = 3;
constexpr int sharePriceColumn = 4;
constexpr int commissionColumn = 5;
constexpr int totalAmountColumn = 6;
constexpr int columnCount = 7;

QDate qDateFromPvDate(pv::Date date) {
  pv::YearMonthDay ymd(date);
  return QDate(static_cast<int>(ymd.year()), static_cast<unsigned int>(ymd.month()),
               static_cast<unsigned int>(ymd.day()));
}

pv::Date pvDateFromQDate(QDate date) {
  return pv::Date(pv::YearMonthDay(pv::Year(date.year()), pv::Month(date.month()), pv::Day(date.day())));
}

pv::Decimal nanDec() { return pv::Decimal(0) / 0; }

} // namespace

pvui::models::TransactionModel::DisplayData
pvui::models::TransactionModel::createDisplayData(const pv::Transaction& transaction) noexcept {
  const auto* action = pvui::actionData(transaction.action());

  switch (transaction.action()) {
  case pv::Action::BUY: {
    const auto& t = static_cast<const pv::BuyTransaction&>(transaction);
    return {t.date,
            action,
            t.security,
            t.numberOfShares,
            t.sharePrice,
            t.commission,
            (t.numberOfShares * t.sharePrice) + t.commission};
  }
  case pv::Action::SELL: {
    const auto& t = static_cast<const pv::SellTransaction&>(transaction);
    return {t.date,
            action,
            t.security,
            t.numberOfShares,
            t.sharePrice,
            t.commission,
            (t.numberOfShares * t.sharePrice) - t.commission};
  }
  case pv::Action::DEPOSIT: {
    const auto& t = static_cast<const pv::DepositTransaction&>(transaction);
    return {t.date, action, t.security, nanDec(), nanDec(), nanDec(), t.amount};
  }
  case pv::Action::WITHDRAW: {
    const auto& t = static_cast<const pv::WithdrawTransaction&>(transaction);
    return {t.date, action, t.security, nanDec(), nanDec(), nanDec(), t.amount};
  }
  case pv::Action::DIVIDEND: {
    const auto& t = static_cast<const pv::DividendTransaction&>(transaction);
    return {t.date, action, t.security, nanDec(), nanDec(), nanDec(), t.amount};
  }
  default: {
    return {transaction.date, action, nullptr, nanDec(), nanDec(), nanDec(), nanDec()};
  }
  }
}

pvui::models::TransactionModel::TransactionModel(pv::DataFile& dataFile, pv::Account& account, QObject* parent)
    : QAbstractTableModel(parent), account_(account), dataFile_(dataFile) {
  assert(dataFile_.owns(account) && "TransactionModel DataFile and account mismatch");
  transactions.reserve(account.transactions().size());
  for (const auto* transaction : account.transactions()) {
    transactions.push_back(createDisplayData(*transaction));
  }

  // Listen for added transactions
  QObject::connect(this, &TransactionModel::transactionAdded, this,
                   [&](std::size_t index, const pv::Transaction& transaction) {
                     beginInsertRows(QModelIndex(), int(index), int(index));
                     transactions.push_back(createDisplayData(transaction));
                     endInsertRows();
                   });

  transactionAddedConnection = account.listenTransactionAdded(
      [&](std::size_t index, const pv::Transaction* transaction) { emit transactionAdded(index, *transaction); });

  // Listen for changed transactions
  QObject::connect(this, &TransactionModel::transactionChanged, this,
                   [&](std::size_t rowIndexSizeT, const pv::Transaction& transaction) {
                     transactions[rowIndexSizeT] = createDisplayData(transaction);

                     auto rowIndex = static_cast<int>(rowIndexSizeT);
                     QModelIndex topLeft = index(rowIndex, 0);
                     QModelIndex bottomRight = index(rowIndex, columnCount(QModelIndex()));

                     emit dataChanged(topLeft, bottomRight);
                   });

  transactionChangedConnection =
      account.listenTransactionReplaced([&](std::size_t index, const pv::Transaction* transaction,
                                            const pv::Transaction*) { emit transactionChanged(index, *transaction); });

  // Listen for removed transactions
  QObject::connect(this, &TransactionModel::transactionRemoved, this, [&](std::size_t rowIndexSizeT) {
    auto rowIndex = static_cast<int>(rowIndexSizeT);
    beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
    transactions.erase(transactions.begin() + rowIndexSizeT);
    endRemoveRows();
  });

  transactionRemovedConnection =
      account.listenTransactionRemoved([&](std::size_t index) { emit transactionRemoved(index); });
}

int pvui::models::TransactionModel::columnCount(const QModelIndex&) const { return ::columnCount; }

QVariant pvui::models::TransactionModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::AccessibleTextRole) {
    return QVariant();
  }

  assert(static_cast<std::size_t>(index.row()) < transactions.size() && "TransactionModel data() index out of range");
  const DisplayData& d = transactions[index.row()];

  switch (index.column()) {
  case dateColumn:
    return qDateFromPvDate(d.date);
  case actionColumn:
    return d.action == nullptr ? QString("") : d.action->name;
  case securityColumn:
    return d.security == nullptr ? QString::fromUtf8("") : QString::fromStdString(d.security->symbol());
  case numberOfSharesColumn: {
    if (d.numberOfShares != d.numberOfShares) { // if NaN
      return QString::fromStdString("");
    }
    return models::util::numberData(d.numberOfShares, role);
  }
  case sharePriceColumn: {
    if (d.sharePrice != d.sharePrice) { // if NaN
      return QString::fromStdString("");
    }
    return models::util::moneyData(d.sharePrice, role);
  }
  case commissionColumn: {
    if (d.commission != d.commission) { // if NaN
      return QString::fromStdString("");
    }
    return models::util::moneyData(d.commission, role);
  }
  case totalAmountColumn: {
    if (d.totalAmount != d.totalAmount) { // if NaN
      return QString::fromStdString("");
    }
    return models::util::moneyData(d.totalAmount, role);
  }
  default:
    return QVariant();
  }
}

Qt::ItemFlags pvui::models::TransactionModel::flags(const QModelIndex& index) const {
  static const Qt::ItemFlags NonEditable = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  static const Qt::ItemFlags Editable = NonEditable | Qt::ItemIsEditable;

  assert(static_cast<std::size_t>(index.row()) < transactions.size() && "TransactionModel flags() index out of range");

  auto* actionData = transactions[index.row()].action;
  if (actionData == nullptr) {
    return NonEditable;
  }

  switch (actionData->action) {
  case pv::Action::BUY: // Fall through
  case pv::Action::SELL: {
    if (index.column() == securityColumn || index.column() == numberOfSharesColumn ||
        index.column() == sharePriceColumn || index.column() == commissionColumn ||
        index.column() == totalAmountColumn) {
      return Editable;
    } else {
      return NonEditable;
    }
  }
  case pv::Action::DEPOSIT:  //  Fall through
  case pv::Action::WITHDRAW: // Fall through
  case pv::Action::DIVIDEND: {
    if (index.column() == securityColumn || index.column() == totalAmountColumn) {
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

  auto transaction = transactions.at(index.row());
  if (!value.canConvert<double>())
    return false;

  std::size_t rowIndex = index.row();

  DisplayData d = transactions[rowIndex]; // Local copy to make changes to

  switch (index.column()) {
  case dateColumn:
    d.date = pvDateFromQDate(value.toDate());
    break;
  case securityColumn:
    d.security = dataFile_.securityForSymbol(value.toString().toStdString());
    break;
  case numberOfSharesColumn:
    d.numberOfShares = pv::Decimal(value.toString().toStdString());
    break;
  case sharePriceColumn:
    d.sharePrice = pv::Decimal(value.toString().toStdString());
    break;
  case commissionColumn:
    d.commission = pv::Decimal(value.toString().toStdString());
    break;
  case totalAmountColumn:
    d.totalAmount = pv::Decimal(value.toString().toStdString());
    break;
  }

  switch (d.action->action) {
  case pv::Action::BUY: {
    return account_.replaceTransaction(
               rowIndex, pv::BuyTransaction(d.date, d.security, d.numberOfShares, d.sharePrice, d.commission)) ==
           pv::TransactionOperationResult::SUCCESS;
  }
  case pv::Action::SELL: {
    return account_.replaceTransaction(
               rowIndex, pv::SellTransaction(d.date, d.security, d.numberOfShares, d.sharePrice, d.commission)) ==
           pv::TransactionOperationResult::SUCCESS;
  }
  case pv::Action::DEPOSIT: {
    return account_.replaceTransaction(rowIndex, pv::DepositTransaction(d.date, d.security, d.totalAmount)) ==
           pv::TransactionOperationResult::SUCCESS;
  }
  case pv::Action::WITHDRAW: {
    return account_.replaceTransaction(rowIndex, pv::WithdrawTransaction(d.date, d.security, d.totalAmount)) ==
           pv::TransactionOperationResult::SUCCESS;
  }
  case pv::Action::DIVIDEND: {
    return account_.replaceTransaction(rowIndex, pv::DividendTransaction(d.date, d.security, d.totalAmount)) ==
           pv::TransactionOperationResult::SUCCESS;
  }
  }
  return false;
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
