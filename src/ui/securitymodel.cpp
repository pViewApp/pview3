#include "SecurityModel.h"

pvui::models::SecurityModel::SecurityModel(pv::DataFile& dataFile, QObject* parent)
    : QAbstractTableModel(parent), dataFile_(dataFile) {
  securityAddedConnection =
      dataFile.securityAdded().connect([&](pv::SecurityPtr security) { emit securityAdded(security); });

  QObject::connect(this, &SecurityModel::securityAdded, this, [&](pv::SecurityPtr security) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    securities.push_back(security);

    endInsertRows();
  });
}

QVariant pvui::models::SecurityModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole || !index.isValid())
    return QVariant();

  const pv::SecurityPtr& security = securities.at(index.row());
  switch (index.column()) {
  case 0:
    return QString::fromStdString(security->symbol());
  case 1:
    return QString::fromStdString(security->name());
  case 2:
    return QString::fromStdString(security->assetClass());
  case 3:
    return QString::fromStdString(security->sector());
  default:
    return QVariant();
  }
}

QVariant pvui::models::SecurityModel::headerData(int section, Qt::Orientation orientation, int role) const {
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
