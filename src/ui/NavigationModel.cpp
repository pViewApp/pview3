#include "NavigationModel.h"
#include <algorithm>

constexpr int accountHeaderRowIndex = 0;
constexpr int reportHeaderRowIndex = 1;
constexpr int securitiesPageRowIndex = 2;

constexpr std::array topLevelIndexes{accountHeaderRowIndex, reportHeaderRowIndex, securitiesPageRowIndex};

constexpr int topLevelRowCount = static_cast<int>(topLevelIndexes.size());

namespace pvui {
namespace models {

void NavigationModel::setupAccount(const pv::AccountPtr account) noexcept {
  accountNameChangedConnections.push_back(account->nameChanged().connect([&](const std::string&, const std::string&) {
    int accountIndex = std::find(accounts.cbegin(), accounts.cend(), account) - accounts.cbegin();
    QModelIndex modelIndex = index(accountIndex, 0, accountsHeaderIndex);
    emit dataChanged(modelIndex, modelIndex);
  }));
}

// Called whenever the dataFile is changed
void NavigationModel::setDataFile(pv::DataFile& dataFile) noexcept {
  // Clear old connections
  accountNameChangedConnections.clear();
  accountNameChangedConnections.reserve(dataFile.accounts().size());

  for (const auto& account : dataFile.accounts()) {
    setupAccount(account);
  };

  // Listen for new account
  accountAddedConnection = dataFile.accountAdded().connect([&](pv::AccountPtr account) { emit accountAdded(account); });

  QObject::connect(this, &NavigationModel::accountAdded, this, [&](pv::AccountPtr account) {
    auto indexToInsertAt = rowCount(accountsHeaderIndex);
    emit beginInsertRows(accountsHeaderIndex, indexToInsertAt, indexToInsertAt);

    accounts.push_back(account);

    emit endInsertRows();
    setupAccount(account);
  });

  // Listen for account removal
  accountRemovedConnection =
      dataFile.accountRemoved().connect([&](pv::AccountPtr account) { emit accountRemoved(account); });

  QObject::connect(this, &NavigationModel::accountRemoved, this, [&](pv::AccountPtr account) {
    auto iter = std::find(accounts.cbegin(), accounts.cend(), account);
    int rowIndex = iter - accounts.cbegin();

    emit beginRemoveRows(accountsHeaderIndex, rowIndex, rowIndex);
    accounts.erase(iter);
    emit endRemoveRows();
  });
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
    return 0; // todo Reports not implemented yet
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
      // todo not implemented
      return QVariant();
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
      return createIndex(row, column, &accountsHeaderIndex);
    } else if (parent == reportsHeaderIndex) {
      return createIndex(row, column, &reportsHeaderIndex);
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

    return accounts.at(index.row())->setName(newAccountName.toStdString());
  } else
    return false; // Not editable by default
}

bool NavigationModel::isAccountsHeader(const QModelIndex& index) const { return index == accountsHeaderIndex; }

bool NavigationModel::isAccountPage(const QModelIndex& index) const {
  return index.internalPointer() == &accountsHeaderIndex;
}

bool NavigationModel::isReportsHeader(const QModelIndex& index) const { return index == reportsHeaderIndex; }

bool NavigationModel::isReportPage(const QModelIndex&) const {
  return false; // Not implemented yet todo
}

bool NavigationModel::isSecuritiesPage(const QModelIndex& index) const { return index == securitiesPageIndex; }

pv::AccountPtr NavigationModel::mapToAccount(const QModelIndex& index) const {
  if (!isAccountPage(index))
    return nullptr;
  return accounts.at(index.row());
}

QModelIndex NavigationModel::mapFromAccount(const pv::AccountPtr account) const {
  int rowIndex = std::find(accounts.cbegin(), accounts.cend(), account) - accounts.cbegin();

  return index(rowIndex, 0, accountsHeaderIndex);
}

} // namespace models
} // namespace pvui
