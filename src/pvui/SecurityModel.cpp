#include "SecurityModel.h"
#include "pv/Security.h"
#include <sqlite3.h>

namespace pvui {
namespace models {

SecurityModel::SecurityModel(pv::DataFile& dataFile, QObject* parent)
    : QAbstractTableModel(parent), dataFile_(dataFile) {
  repopulate();

  securityAddedConnection = dataFile.onSecurityAdded([&](pv::i64 security) { emit securityAdded(security); });

  QObject::connect(this, &SecurityModel::securityAdded, this, [&](pv::i64 security) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    securities.push_back(security);
    endInsertRows();
  });

  securityRemovedConnection =
      dataFile.onSecurityRemoved([&](const pv::i64 security) { emit securityRemoved(security); });

  QObject::connect(this, &SecurityModel::securityRemoved, this, [&](pv::i64 security) {
    auto iter = std::find(securities.cbegin(), securities.cend(), security);
    if (iter == securities.end())
      return;

    int rowIndex = iter - securities.cbegin();

    beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
    securities.erase(std::find(securities.cbegin(), securities.cend(), security));
    endRemoveRows();
  });

  resetConnection = dataFile_.onRollback([&]() { emit reset(); });

  QObject::connect(this, &SecurityModel::reset, this, [&]() {
    beginResetModel();
    repopulate();
    endResetModel();
  });
}

void SecurityModel::repopulate() {
  securities.clear();
  auto stmt = dataFile_.query("SELECT Id FROM SECURITIES");
  while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
    securities.push_back(sqlite3_column_int64(stmt.get(), 0));
  }
}

QVariant SecurityModel::data(const QModelIndex& index, int role) const {
  if ((role != Qt::DisplayRole && role != Qt::AccessibleTextRole && role != Qt::EditRole) || !index.isValid())
    return QVariant();

  const pv::i64 security = securities.at(index.row());
  switch (index.column()) {
  case 0:
    return QString::fromStdString(pv::security::symbol(dataFile_, security));
  case 1:
    return QString::fromStdString(pv::security::name(dataFile_, security));
  case 2:
    return QString::fromStdString(pv::security::assetClass(dataFile_, security));
  case 3:
    return QString::fromStdString(pv::security::sector(dataFile_, security));
  default:
    return QVariant();
  }
}

QVariant SecurityModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Orientation::Horizontal)
    return QVariant();
  if (role != Qt::DisplayRole)
    return QVariant();

  switch (section) {
  case 0:
    return tr("Symbol");
  case 1:
    return tr("Name");
  case 2:
    return tr("Asset Class");
  case 3:
    return tr("Sector");
  default:
    return QVariant();
  }
}

Qt::ItemFlags SecurityModel::flags(const QModelIndex& index) const {
  if (index.column() == 0) {
    // Symbol column
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren; // Symbol not editable
  } else
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEditable;
}

bool SecurityModel::setData(const QModelIndex& index, const QVariant& data, int role) {
  if (role != Qt::EditRole)
    return false;
  if (!data.canConvert<QString>())
    return false; // All editable fields are strings

  std::string newData = data.toString().trimmed().toStdString();

  if (newData.length() == 0)
    return false; // Must not be empty

  pv::i64 security = securities.at(index.row());

  switch (index.column()) {
  case 0:
    return false; // Symbol not editable
  case 1:
    dataFile_.setSecurityName(security, newData);
    return true;
  case 2:
    dataFile_.setSecurityAssetClass(security, newData);
    return true;
  case 3:
    dataFile_.setSecuritySector(security, newData);
    return true;
  default:
    return false;
  }
}

int SecurityModel::rowOfSecurity(pv::i64 security) const noexcept {
  auto iter = std::find(securities.cbegin(), securities.end(), security);
  if (iter == securities.end()) {
    return -1;
  }
  return static_cast<int>(iter - securities.cbegin());
}

pv::i64 SecurityModel::securityOfRow(int rowIndex) const noexcept {
  auto iter = securities.cbegin() + rowIndex;

  if (iter == securities.end())
    return -1;
  return *iter;
}

} // namespace models
} // namespace pvui
