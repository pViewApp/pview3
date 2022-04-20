#include "SecurityModel.h"

pvui::models::SecurityModel::SecurityModel(pv::DataFile& dataFile) : dataFile_(dataFile) {
  // Before security_ added

  QObject::connect(this, &SecurityModel::beforeSecurityAdded, this, &SecurityModel::beginInsertRows);
  beforeSecurityAddedConnection = dataFile.beforeSecurityAdded().connect(
      [&]() { emit beforeSecurityAdded(QModelIndex(), rowCount(), rowCount()); });

  // After security_ added

  QObject::connect(this, &SecurityModel::afterSecurityAdded, this, &SecurityModel::endInsertRows);
  afterSecurityAddedConnection = dataFile.securityAdded().connect([&](pv::SecurityPtr) { emit afterSecurityAdded(); });
}

QVariant pvui::models::SecurityModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole || index.internalPointer() == nullptr || !index.isValid())
    return QVariant();

  pv::Security* security = static_cast<pv::Security*>(index.internalPointer());
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
