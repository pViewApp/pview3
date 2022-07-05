#ifndef PVUI_MODELS_SECURITY_MODEL_H
#define PVUI_MODELS_SECURITY_MODEL_H

#include "pv/DataFile.h"
#include "pv/Signals.h"
#include <QAbstractTableModel>
#include <QObject>
#include <optional>
#include <vector>

namespace pvui::models {
class SecurityModel : public QAbstractTableModel {
  Q_OBJECT
private:
  pv::DataFile& dataFile_;
  std::vector<pv::i64> securities;

  // Connections
  pv::ScopedConnection securityAddedConnection;
  pv::ScopedConnection securityRemovedConnection;
  pv::ScopedConnection securityUpdatedConnection;
  pv::ScopedConnection resetConnection;

  void setupSecurity(pv::i64 security);

  void repopulate();
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

  int rowOfSecurity(pv::i64 security) const noexcept;
  pv::i64 securityOfRow(int rowIndex) const noexcept;
signals:
  void reset();
  void securityAdded(pv::i64 security);
  void securityRemoved(pv::i64 security);
};
} // namespace pvui::models

#endif // PVUI_MODELS_SECURITY_MODEL_H
