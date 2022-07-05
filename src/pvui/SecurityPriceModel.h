#ifndef PVUI_MODELS_SECURITYPRICEMODEL_H
#define PVUI_MODELS_SECURITYPRICEMODEL_H

#include "pv/DataFile.h"
#include "pv/Security.h"
#include <QDate>
#include "pv/Integer64.h"
#include <QAbstractTableModel>
#include "pv/Signals.h"
#include <optional>
#include <QAbstractTableModel>
#include <set>
#include <vector>

namespace pvui::models {

class SecurityPriceModel : public QAbstractTableModel {
  Q_OBJECT
private:
  pv::DataFile& dataFile;
  std::vector<pv::i64> dates;
  pv::i64 security;

  pv::ScopedConnection securityPriceUpdatedConnection;
  pv::ScopedConnection securityPriceRemovedConnection;
  pv::ScopedConnection resetConnection;

  void repopulate();
public:
  SecurityPriceModel(pv::DataFile& dataFile, pv::i64 security, QObject* parent = nullptr);

  QDate mapToDate(const QModelIndex& index) const noexcept;
  std::optional<QModelIndex> mapFromDate(QDate date);

  // Overrides

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
signals:
  void dateUpdated(pv::i64 date);
  void dateRemoved(pv::i64 date);
  void reset();
};

} // namespace pvui::models

#endif // PVUI_MODELS_SECURITYPRICEMODEL_H
