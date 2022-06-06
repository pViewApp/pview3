#ifndef PVUI_MODELS_HOLDINGSMODEL_H
#define PVUI_MODELS_HOLDINGSMODEL_H

#include "pv/DataFile.h"
#include "pv/Signals.h"
#include <QAbstractTableModel>
#include <unordered_map>

namespace pvui {
namespace models {

class HoldingsModel : public QAbstractTableModel {
  Q_OBJECT
private:
  pv::DataFile& dataFile_;
  std::vector<pv::Security*> securities;

  // Connections
  boost::signals2::scoped_connection securityAddedConnection;
  boost::signals2::scoped_connection securityRemovedConnection;
  std::unordered_multimap<pv::Security*, pv::ScopedConnection> securityChangeConnections;

  void update(pv::Security& security, int column);

protected:
  void createListeners(pv::Security& security);
  void removeListeners(pv::Security& security);

public:
  explicit HoldingsModel(pv::DataFile& dataFile, QObject* parent = nullptr);

  int rowCount(const QModelIndex& = QModelIndex()) const override;
  int columnCount(const QModelIndex& = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
signals:
  void securityAdded(pv::Security& security);
  void securityRemoved(const pv::Security& security);
  void securityChanged(pv::Security& security);

  // QAbstractItemModel interface
};

} // namespace models
} // namespace pvui

#endif // PVUI_MODELS_HOLDINGSMODEL_H
