#include "NavigationModel.h"
#include <algorithm>

constexpr int accountHeaderRowIndex = 0;
constexpr int reportHeaderRowIndex = 1;
constexpr int securitiesPageRowIndex = 2;

constexpr std::array topLevelIndexes{accountHeaderRowIndex, reportHeaderRowIndex, securitiesPageRowIndex};

constexpr int topLevelRowCount = static_cast<int>(topLevelIndexes.size());

namespace pvui {
namespace models {

void NavigationModel::setupAccount(pv::Account* account) noexcept {
  accountNameChangedConnections.push_back(account->listenNameChanged([&](std::string, std::string) {
    int accountIndex = std::find(accounts.cbegin(), accounts.cend(), account) - accounts.cbegin();
    QModelIndex modelIndex = index(accountIndex, 0, accountsHeaderIndex);
    emit dataChanged(modelIndex, modelIndex);
  }));
}

// Called whenever the dataFile is changed
void NavigationModel::setDataFile(pv::DataFile& dataFile) noexcept {
  beginResetModel();

  accounts.clear();
  // Clear old connections
  accountNameChangedConnections.clear();
  accountNameChangedConnections.reserve(dataFile.accounts().size());

  for (const auto& account : dataFile.accounts()) {
    setupAccount(account);
  };

  // Listen for new account
  accountAddedConnection = dataFile.listenAccountAdded([&](pv::Account* account) { emit accountAdded(account); });

  QObject::connect(this, &NavigationModel::accountAdded, this, [&](pv::Account* account) {
    auto indexToInsertAt = rowCount(accountsHeaderIndex);
    beginInsertRows(accountsHeaderIndex, indexToInsertAt, indexToInsertAt);

    accounts.push_back(account);

    endInsertRows();
    setupAccount(account);
  });

  // Listen for account removal
  accountRemovedConnection =
      dataFile.listenAccountRemoved([&](const pv::Account* account) { emit accountRemoved(account); });

  QObject::connect(this, &NavigationModel::accountRemoved, this, [&](const pv::Account* account) {
    auto iter = std::find(accounts.cbegin(), accounts.cend(), account);
    int rowIndex = iter - accounts.cbegin();

    beginRemoveRows(accountsHeaderIndex, rowIndex, rowIndex);
    accounts.erase(iter);
    endRemoveRows();
  });

  endResetModel();
}

NavigationModel::NavigationModel(pvui::DataFileManager& dataFileManager, QObject* parent)
    : QAbstractItemModel{parent}, dataFileManager_(dataFileManager) {
  accountsHeaderIndex = createIndex(accountHeaderRowIndex, 0);
  reportsHeaderIndex = createIndex(reportHeaderRowIndex, 0);
  securitiesPageIndex = createIndex(securitiesPageRowIndex, 0);

  QObject::connect(&dataFileManager, &DataFileManager::dataFileChanged, this, &NavigationModel::setDataFile);
  setDataFile(*dataFileManager);
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
      return QString::fromStdString(accounts.at(index.row())->name());
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

    accounts.at(index.row())->setName(newAccountName.toStdString());
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

pv::Account* NavigationModel::accountFromIndex(const QModelIndex& index) const {
  if (!isAccountPage(index))
    return nullptr;
  return accounts.at(index.row());
}

QModelIndex NavigationModel::accountToIndex(const pv::Account& account) const {
  int rowIndex = std::find(accounts.cbegin(), accounts.cend(), &account) - accounts.cbegin();

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
