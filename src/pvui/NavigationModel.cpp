#include "NavigationModel.h"
#include "pv/Account.h"
#include "pv/Integer64.h"
#include <algorithm>
#include <sqlite3.h>
#include <array>

constexpr int accountHeaderRowIndex = 0;
constexpr int reportHeaderRowIndex = 1;
constexpr int securitiesPageRowIndex = 2;

constexpr std::array topLevelIndexes{accountHeaderRowIndex, reportHeaderRowIndex, securitiesPageRowIndex};

constexpr int topLevelRowCount = static_cast<int>(topLevelIndexes.size());

namespace pvui {
namespace models {

void NavigationModel::repopulateAccounts() {
  accounts.clear();
  if (!dataFileManager_.has()) {
    return;
  }
  auto query = dataFileManager_->query("SELECT Id FROM Accounts");
  while (sqlite3_step(&*query) == SQLITE_ROW) {
    accounts.push_back(sqlite3_column_int64(&*query, 0));
  }
}

void NavigationModel::handleDataFileChanged() noexcept {
  beginResetModel();

  repopulateAccounts();

  endResetModel();

  accountAddedConnection.disconnect();
  accountUpdatedConnection.disconnect();
  accountRemovedConnection.disconnect();
  resetConnection.disconnect();
  
  if (!dataFileManager_.has()) {
    return;
  }

  // Listen for new account
  accountAddedConnection = dataFileManager_->onAccountAdded([&](pv::i64 account) { emit accountAdded(account); });

  // Listen for account removal
  accountRemovedConnection =
      dataFileManager_->onAccountRemoved([&](const pv::i64 account) { emit accountRemoved(account); });

  accountUpdatedConnection = dataFileManager_->onAccountUpdated([&](pv::i64 account) { emit accountUpdated(account); });

  resetConnection = dataFileManager_->onRollback([&] { emit reset(); });
}

NavigationModel::NavigationModel(pvui::DataFileManager& dataFileManager, QObject* parent)
    : QAbstractItemModel{parent}, dataFileManager_(dataFileManager) {
  accountsHeaderIndex = createIndex(accountHeaderRowIndex, 0);
  reportsHeaderIndex = createIndex(reportHeaderRowIndex, 0);
  securitiesPageIndex = createIndex(securitiesPageRowIndex, 0);

  //// Listen account add, update, remove, and dataFile rollback
  QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this, &NavigationModel::handleDataFileChanged);

  QObject::connect(this, &NavigationModel::accountAdded, this, [&](pv::i64 account) {
    auto indexToInsertAt = rowCount(accountsHeaderIndex);
    beginInsertRows(accountsHeaderIndex, indexToInsertAt, indexToInsertAt);

    accounts.push_back(account);

    endInsertRows();
  });

  QObject::connect(this, &NavigationModel::accountRemoved, this, [&](pv::i64 account) {
    auto iter = std::find(accounts.cbegin(), accounts.cend(), account);
    int rowIndex = iter - accounts.cbegin();

    beginRemoveRows(accountsHeaderIndex, rowIndex, rowIndex);
    accounts.erase(iter);
    endRemoveRows();
  });

  QObject::connect(this, &NavigationModel::accountUpdated, this, [&](pv::i64 account) {
    int rowIndex = static_cast<int>(std::find(accounts.cbegin(), accounts.cend(), account) - accounts.cbegin());
    auto modelIndex = index(rowIndex, 0, QModelIndex());
    emit dataChanged(modelIndex, modelIndex);
  });

  QObject::connect(this, &NavigationModel::reset, this, &NavigationModel::repopulateAccounts);

  handleDataFileChanged();
}

int NavigationModel::rowCount(const QModelIndex& parent) const {
  if (!parent.isValid())
    return topLevelRowCount;

  if (parent.parent().isValid()) {
    return 0; // No children for not top-level items
  }

  switch (parent.row()) {
  case accountHeaderRowIndex: {
    return static_cast<int>(accounts.size());
  }
  case reportHeaderRowIndex: {
    return static_cast<int>(reports.size());
  }
  case securitiesPageRowIndex: {
    return 0;
  }
  default: {
    return 0;
  }
  }
}

QVariant NavigationModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();

  if (!index.parent().isValid()) {
    // Top-level row

    switch (index.row()) {
    case accountHeaderRowIndex: {
      return tr("Accounts");
    }
    case reportHeaderRowIndex: {
      return tr("Reports");
    }
    case securitiesPageRowIndex: {
      return tr("Securities");
    }
    default: {
      return QVariant();
    }
    }
  } else {
    // Sub-row
    if (index.parent().row() == accountHeaderRowIndex) {
      // Account row
      return QString::fromStdString(pv::account::name(*dataFileManager_, accountFromIndex(index)));
    } else if (index.parent().row() == reportHeaderRowIndex) {
      return reportFromIndex(index)->name();
    }
  }

  return QVariant();
}

QModelIndex NavigationModel::index(int row, int column, const QModelIndex& parent) const {
  if (column != 0) {
    return QModelIndex();
  }

  if (parent.isValid()) {
    if (parent == accountsHeaderIndex) {
      return createIndex(row, column, const_cast<QModelIndex*>(&accountsHeaderIndex));
    } else if (parent == reportsHeaderIndex) {
      return createIndex(row, column, const_cast<QModelIndex*>(&reportsHeaderIndex));
    } else {
      return QModelIndex();
    }
  }

  switch (row) {
  case accountHeaderRowIndex: {
    return accountsHeaderIndex;
  }
  case reportHeaderRowIndex: {
    return reportsHeaderIndex;
  }
  case securitiesPageRowIndex: {
    return securitiesPageIndex;
  }
  default:
    return QModelIndex();
  }
}

QModelIndex NavigationModel::parent(const QModelIndex& index) const {
  if (isAccountPage(index)) {
    return accountsHeaderIndex;
  } else if (isReportPage(index)) {
    return reportsHeaderIndex;
  }
  return QModelIndex();
}

Qt::ItemFlags NavigationModel::flags(const QModelIndex& index) const {
  static Qt::ItemFlags baseFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  static Qt::ItemFlags accountFlags = baseFlags | Qt::ItemIsEditable;

  if (isAccountPage(index)) {
    return accountFlags;
  } else if (isAccountsHeader(index) || isReportsHeader(index)) {
    return baseFlags;
  } else {
    return baseFlags | Qt::ItemNeverHasChildren;
  }
}

bool NavigationModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (role != Qt::EditRole)
    return false;
  if (isAccountPage(index)) {
    if (!value.canConvert<QString>())
      return false;

    auto newAccountName = value.toString().trimmed();
    if (newAccountName.isEmpty())
      return false;

    dataFileManager_->setAccountName(accountFromIndex(index), newAccountName.toStdString());
    return true;
  } else
    return false; // Not editable by default
}

bool NavigationModel::isAccountsHeader(const QModelIndex& index) const { return index == accountsHeaderIndex; }

bool NavigationModel::isAccountPage(const QModelIndex& index) const {
  return index.internalPointer() == &accountsHeaderIndex;
}

bool NavigationModel::isReportsHeader(const QModelIndex& index) const { return index == reportsHeaderIndex; }

bool NavigationModel::isReportPage(const QModelIndex& index) const {
  return index.internalPointer() == &reportsHeaderIndex;
}

bool NavigationModel::isSecuritiesPage(const QModelIndex& index) const { return index == securitiesPageIndex; }

pv::i64 NavigationModel::accountFromIndex(const QModelIndex& index) const {
  if (!isAccountPage(index))
    return -1;
  return accounts.at(index.row());
}

QModelIndex NavigationModel::accountToIndex(pv::i64 account) const {
  int rowIndex = std::find(accounts.cbegin(), accounts.cend(), account) - accounts.cbegin();

  return index(rowIndex, 0, accountsHeaderIndex);
}

const Report* NavigationModel::reportFromIndex(const QModelIndex& index) const {
  if (!isReportPage(index))
    return nullptr;
  return reports.at(index.row());
}

void NavigationModel::addReport(const Report* report) {
  beginInsertRows(reportsHeaderIndex, rowCount(reportsHeaderIndex), rowCount(reportsHeaderIndex));
  reports.push_back(report);
  endInsertRows();
}

void NavigationModel::addReports(const std::vector<Report*>& reportVector) {
  auto first = rowCount(reportsHeaderIndex);
  auto last = first + static_cast<int>(reportVector.size());
  beginInsertRows(reportsHeaderIndex, first, last);
  this->reports.insert(this->reports.end(), reportVector.cbegin(), reportVector.cend());
  endInsertRows();
}

void NavigationModel::removeReport(const Report* report) {
  auto iter = std::find(reports.cbegin(), reports.cend(), report);
  auto rowIndex = iter - reports.cbegin();

  if (iter == reports.cend()) {
    return;
  }

  beginRemoveRows(reportsHeaderIndex, rowIndex, rowIndex);
  reports.erase(iter);
  endRemoveRows();
}

} // namespace models
} // namespace pvui
