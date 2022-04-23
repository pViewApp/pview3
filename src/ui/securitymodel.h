#ifndef PVUI_MODELS_SECURITY_MODEL_H
#define PVUI_MODELS_SECURITY_MODEL_H

#include "DataFile.h"
#include <QAbstractTableModel>
#include <QObject>
#include <boost/signals2.hpp>
#include <vector>

namespace pvui::models {
class SecurityModel : public QAbstractTableModel {
  Q_OBJECT
private:
  pv::DataFile& dataFile_;
  boost::signals2::scoped_connection securityAddedConnection;

  std::vector<pv::SecurityPtr> securities;

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
signals:
  void securityAdded(pv::SecurityPtr security);
};
} // namespace pvui::models

#endif // PVUI_MODELS_SECURITY_MODEL_H
