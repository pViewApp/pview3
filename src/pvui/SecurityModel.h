#ifndef PVUI_MODELS_SECURITY_MODEL_H
#define PVUI_MODELS_SECURITY_MODEL_H

#include "pv/DataFile.h"
#include "pv/Signals.h"
#include <QAbstractTableModel>
#include <QObject>
#include <map>
#include <vector>

namespace pvui::models {
class SecurityModel : public QAbstractTableModel {
  Q_OBJECT
private:
  pv::DataFile& dataFile_;
  std::vector<pv::Security*> securities;

  // Connections
  pv::ScopedConnection securityAddedConnection;
  pv::ScopedConnection securityRemovedConnection;
  std::unordered_multimap<const pv::Security*, boost::signals2::scoped_connection> securityChangeConnections;

  void setupSecurity(pv::Security* security);

public:
  SecurityModel(pv::DataFile& dataFile, QObject* parent = nullptr);

  int rowCount(const QModelIndex& = QModelIndex()) const override { return static_cast<int>(securities.size()); }

  int columnCount(const QModelIndex& = QModelIndex()) const override {
    return 4; // Symbol, name, asset class, sector
  }

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  bool setData(const QModelIndex& index, const QVariant& data, int role) override;

  const pv::Security* mapFromIndex(const QModelIndex& index) const noexcept;
  pv::Security* mapFromIndex(const QModelIndex& index) noexcept;
signals:
  void securityAdded(pv::Security* security);
  void securityRemoved(const pv::Security* security);
};
} // namespace pvui::models

#endif // PVUI_MODELS_SECURITY_MODEL_H
