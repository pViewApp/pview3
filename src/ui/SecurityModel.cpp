#include "SecurityModel.h"

void pvui::models::SecurityModel::setupSecurity(pv::SecurityPtr security) {
  auto slot = [&](std::string, std::string) {
    int rowIndex = std::find(securities.cbegin(), securities.cend(), security) - securities.cbegin();
    QModelIndex firstModelIndex = index(rowIndex, 0);
    QModelIndex lastModelIndex = index(rowIndex, columnCount());
    emit dataChanged(firstModelIndex, lastModelIndex);
  };

  securityChangeConnections.insert({security, security->nameChanged().connect(slot)});
  securityChangeConnections.insert({security, security->assetClassChanged().connect(slot)});
  securityChangeConnections.insert({security, security->sectorChanged().connect(slot)});
}

pvui::models::SecurityModel::SecurityModel(pv::DataFile& dataFile, QObject* parent)
    : QAbstractTableModel(parent), dataFile_(dataFile) {
  securityAddedConnection =
      dataFile.securityAdded().connect([&](pv::SecurityPtr security) { emit securityAdded(security); });

  QObject::connect(this, &SecurityModel::securityAdded, this, [&](pv::SecurityPtr security) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    securities.push_back(security);
    endInsertRows();

    setupSecurity(security);
  });

  securityRemovedConnection =
      dataFile.securityRemoved().connect([&](pv::SecurityPtr security) { emit securityRemoved(security); });

  QObject::connect(this, &SecurityModel::securityRemoved, this, [&](pv::SecurityPtr security) {
    auto iter = std::find(securities.cbegin(), securities.cend(), security);
    if (iter == securities.end())
      return;

    int rowIndex = iter - securities.cbegin();

    beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
    securities.erase(std::find(securities.cbegin(), securities.cend(), security));
    endRemoveRows();

    securityChangeConnections.erase(security);
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

Qt::ItemFlags pvui::models::SecurityModel::flags(const QModelIndex& index) const {
  if (index.column() == 0) {
    // Symbol column
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren; // Symbol not editable
  } else
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEditable;
}

bool pvui::models::SecurityModel::setData(const QModelIndex& index, const QVariant& data, int role) {
  if (role != Qt::EditRole)
    return false;
  if (!data.canConvert<QString>())
    return false; // All editable fields are strings

  std::string newData = data.toString().trimmed().toStdString();

  if (newData.length() == 0)
    return false; // Must not be empty

  pv::SecurityPtr security = securities.at(index.row());

  switch (index.column()) {
  case 0:
    return false; // Symbol not editable
  case 1:
    return security->setName(newData);
  case 2:
    return security->setAssetClass(newData);
  case 3:
    return security->setSector(newData);
  default:
    return false;
  }
}

pv::SecurityPtr pvui::models::SecurityModel::mapFromIndex(const QModelIndex& index) const {
  auto iter = securities.cbegin() + index.row();

  if (iter == securities.end())
    return nullptr;
  return *iter;
}
