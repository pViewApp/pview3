#ifndef PVUI_MODELS_HOLDINGSMODEL_H
#define PVUI_MODELS_HOLDINGSMODEL_H

#include "pv/DataFile.h"
#include "pv/Integer64.h"
#include "pv/Signals.h"
#include <QAbstractTableModel>
#include <unordered_map>

class sqlite3_stmt;
namespace pvui {
namespace models {

class HoldingsModel : public QAbstractTableModel {
  Q_OBJECT
private:
  sqlite3_stmt* stmt_rowNumberOfSecurity = nullptr;

  pv::DataFile& dataFile_;
  std::vector<pv::i64> securities;

  // Connections
  pv::ScopedConnection changedConnection;
  pv::ScopedConnection resetConnection;

  void update(pv::i64 security);

  void repopulate();
public:
  explicit HoldingsModel(pv::DataFile& dataFile, QObject* parent = nullptr);

  int rowCount(const QModelIndex& = QModelIndex()) const override;
  int columnCount(const QModelIndex& = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
signals:
  void reset();

  // QAbstractItemModel interface
};

} // namespace models
} // namespace pvui

#endif // PVUI_MODELS_HOLDINGSMODEL_H
