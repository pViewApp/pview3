#include "SecurityPriceModel.h"
#include <QDate>
#include <QThread>

pvui::models::SecurityPriceModel::SecurityPriceModel(pv::SecurityPtr security, QObject* parent)
    : QAbstractItemModel(parent), security_(security) {
  dates.reserve(security_->prices().size());
  for (const auto& pair : security_->prices()) {
    dates.push_back(pair.first);
  }

  QObject::connect(this, &SecurityPriceModel::dateAdded, this, [&](pv::Date date) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    dates.push_back(date);
    endInsertRows();
  });

  QObject::connect(this, &SecurityPriceModel::dateRemoved, this, [&](pv::Date date) {
    auto iter = std::find(dates.cbegin(), dates.cend(), date);
    if (iter == dates.cend())
      return;

    int index = static_cast<int>(dates.cbegin() - dates.cbegin());
    beginRemoveRows(QModelIndex(), index, index);
    dates.erase(iter);
    endRemoveRows();
  });

  securityPriceAddedConnection = security_->priceChanged().connect(
      [&](const pv::Date& date, std::optional<pv::Decimal> before, std::optional<pv::Decimal> after) {
        if (!before.has_value() && after.has_value()) {
          // Security Price added
          emit dateAdded(date);
        } else if (before.has_value() && !after.has_value()) {
          // Security Price removed
          emit dateRemoved(date);
        } else if (before.has_value() && after.has_value()) {
          auto row = std::find(dates.cbegin(), dates.cend(), date) - dates.cbegin();
          auto index_ = index(row, 1); // Update the price column
          emit dataChanged(index_, index_);
        } else {
          // Some other case, ignore
        }
      });
}

Qt::ItemFlags pvui::models::SecurityPriceModel::flags(const QModelIndex& index) const {

  if (index.column() == 0)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
  if (index.column() == 1)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEditable;
  else
    return QAbstractItemModel::flags(index);
}

QVariant pvui::models::SecurityPriceModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid())
    return QVariant();
  if (index.row() >= rowCount())
    return QVariant();

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    const pv::Date date = dates.at(index.row());

    switch (index.column()) {
    case 0: {
      pv::YearMonthDay ymd(date);
      return QDate(static_cast<int>(ymd.year()), static_cast<unsigned int>(ymd.month()),
                   static_cast<unsigned int>(ymd.day()));
    }
    case 1: {
      return double{security_->prices().at(date)};
    }
    default: {
      return QVariant();
    }
    }
  }

  return QVariant();
}

QVariant pvui::models::SecurityPriceModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

bool pvui::models::SecurityPriceModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid() || static_cast<std::size_t>(index.row()) >= dates.size())
    return false;
  if (role != Qt::EditRole)
    return false;
  if (index.column() != 1)
    return false; // The price column is editable, the date is not
  if (!value.canConvert<double>())
    return false; // The new value must be a number
  auto price = value.toDouble();

  if (price < 0)
    return false; // Prices should be positive

  security_->setPrice(dates.at(index.row()), price);
  return true; // The dataChanged() signal is emitted via the slot in the
               // constructor
}
