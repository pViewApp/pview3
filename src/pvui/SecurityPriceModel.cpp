#include "SecurityPriceModel.h"
#include "ModelUtils.h"
#include "pv/DataFile.h"
#include <Qt>
#include <QDate>
#include <sqlite3.h>
#include <QThread>
#include <optional>
#include "DateUtils.h"
#include "pv/Security.h"
#include "pvui/DataFileManager.h"

namespace pvui {
namespace models {

namespace {

constexpr int dateColumnIndex = 0;
constexpr int priceColumnIndex = 1;
constexpr int modelColumnCount = 2;

} // namespace

SecurityPriceModel::SecurityPriceModel(pv::DataFile& dataFile, pv::i64 security, QObject* parent)
    : QAbstractTableModel(parent), dataFile(dataFile), security(security) {
  repopulate();

  QObject::connect(this, &SecurityPriceModel::dateUpdated, this, [&](pv::i64 date) {
    auto iter = std::find(dates.cbegin(), dates.cend(), date);
    if (iter != dates.cend()) {
      // date already exists, update it
      auto modelIndex = index(static_cast<int>(iter - dates.cbegin()), priceColumnIndex);
      emit dataChanged(modelIndex, modelIndex);
    } else {
      // date doesn't exist, add it
      beginInsertRows(QModelIndex(), rowCount(), rowCount());
      dates.push_back(date);
      endInsertRows();
    }
  });

  QObject::connect(this, &SecurityPriceModel::dateRemoved, this, [&](pv::i64 date) {
    auto iter = std::find(dates.cbegin(), dates.cend(), date);
    if (iter == dates.cend())
      return;

    int index = static_cast<int>(dates.cbegin() - dates.cbegin());
    beginRemoveRows(QModelIndex(), index, index);
    dates.erase(iter);
    endRemoveRows();
  });

  QObject::connect(this, &SecurityPriceModel::reset, this, [&]() {
    beginResetModel();
    repopulate();
    endResetModel();
  });

  securityPriceUpdatedConnection = dataFile.onSecurityPriceUpdated([&](pv::i64 security, pv::i64 date) {
    if (security == this->security) {
      emit dateUpdated(date);
    }
  });

  securityPriceRemovedConnection = dataFile.onSecurityPriceRemoved([&](pv::i64 security, pv::i64 date) {
    if (security == this->security) {
      emit dateRemoved(date);
    }
  });

  resetConnection = dataFile.onRollback([&] { emit reset(); });
}

void SecurityPriceModel::repopulate() {
  dates.clear();

  auto query = dataFile.query("SELECT Date FROM SecurityPrices WHERE SecurityId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite_int64>(security));
  while (sqlite3_step(&*query) == SQLITE_ROW) {
    dates.push_back(sqlite3_column_int64(&*query, 0));
  }
}

QDate SecurityPriceModel::mapToDate(const QModelIndex& index) const noexcept {
  if (static_cast<std::size_t>(index.row()) <= dates.size()) {
    return toQDate(dates.at(index.row()));
  } else {
    return QDate();
  }
}

std::optional<QModelIndex> SecurityPriceModel::mapFromDate(QDate date) {
  auto iter = std::find(dates.cbegin(), dates.cend(), toEpochDate(date));
  if (iter == dates.cend()) {
    return std::nullopt;
  } else {
    return index(static_cast<int>(iter - dates.cbegin()), 0);
  }
}

int SecurityPriceModel::rowCount(const QModelIndex& parent) const {
  if (!parent.isValid()) {
    return static_cast<int>(dates.size());
  } else {
    return 0;
  }
}

int SecurityPriceModel::columnCount(const QModelIndex&) const { return modelColumnCount; }

Qt::ItemFlags SecurityPriceModel::flags(const QModelIndex& index) const {
  if (index.column() == dateColumnIndex)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
  if (index.column() == priceColumnIndex)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEditable;
  else
    return QAbstractItemModel::flags(index);
}

QVariant SecurityPriceModel::data(const QModelIndex& index, int role) const {
  if (role == Qt::TextAlignmentRole) {
    if (index.column() == priceColumnIndex) {
      return QVariant(Qt::AlignRight | Qt::AlignVCenter); // Right-align numeric column
    } else {
      return QVariant();
    }
  }
  if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::AccessibleTextRole) {
    const pv::i64 date = dates.at(index.row());

    switch (index.column()) {
    case dateColumnIndex: {
      return toQDate(date);
    }
    case priceColumnIndex: {
      return modelutils::moneyData(pv::security::price(dataFile, security, date).value_or(0), role);
    }
    default: {
      return QVariant();
    }
    }
  }

  return QVariant();
}

QVariant SecurityPriceModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole) {
    if (section == 0)
      return tr("Date");
    else if (section == 1)
      return tr("Price");
    else
      return QAbstractItemModel::headerData(section, orientation, role);
  } else {
    return QAbstractItemModel::headerData(section, orientation, role);
  }
}

bool SecurityPriceModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid() || static_cast<std::size_t>(index.row()) >= dates.size())
    return false;
  if (role != Qt::EditRole)
    return false;
  if (index.column() != 1)
    return false; // The price column is editable, the date is not
  if (!value.canConvert<double>())
    return false; // The new value must be a number
  auto price = value.toString();

  if (price < 0)
    return false; // Prices should be positive

  dataFile.setSecurityPrice(security, dates.at(index.row()), price.toDouble() * 100);
  return true; // The dataChanged() signal is emitted via the slot in the
               // constructor
}

} // namespace models
} // namespace pvui
