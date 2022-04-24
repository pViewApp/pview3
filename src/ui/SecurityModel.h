#ifndef PVUI_MODELS_SECURITY_MODEL_H
#define PVUI_MODELS_SECURITY_MODEL_H

#include "DataFile.h"
#include <QAbstractTableModel>
#include <QObject>
#include <boost/signals2/connection.hpp>
#include <map>
#include <vector>

namespace pvui::models {
class SecurityModel : public QAbstractTableModel {
  Q_OBJECT
private:
  pv::DataFile& dataFile_;
  std::vector<pv::SecurityPtr> securities;

  // Connections
  boost::signals2::scoped_connection securityAddedConnection;
  boost::signals2::scoped_connection securityRemovedConnection;
  std::multimap<pv::SecurityPtr, boost::signals2::scoped_connection> securityChangeConnections;

  void setupSecurity(pv::SecurityPtr security);

public:
  SecurityModel(pv::DataFile& dataFile, QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override {
    if (parent.isValid())
      return 0;
    return static_cast<int>(securities.size());
  }

  int columnCount(const QModelIndex& = QModelIndex()) const override {
    return 4; // Symbol, name, asset class, sector
  }

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  bool setData(const QModelIndex& index, const QVariant& data, int role) override;

  pv::SecurityPtr mapFromIndex(const QModelIndex& index) const;
signals:
  void securityAdded(pv::SecurityPtr security);
  void securityRemoved(pv::SecurityPtr security);
};
} // namespace pvui::models

#endif // PVUI_MODELS_SECURITY_MODEL_H
