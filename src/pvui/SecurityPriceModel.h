#ifndef PVUI_MODELS_SECURITYPRICEMODEL_H
#define PVUI_MODELS_SECURITYPRICEMODEL_H

#include "pv/Decimal.h"
#include "pv/Security.h"
#include <QAbstractItemModel>
#include <boost/signals2.hpp>
#include <set>
#include <vector>

namespace pvui::models {

class SecurityPriceModel : public QAbstractItemModel {
  Q_OBJECT
private:
  std::vector<pv::Date> dates;
  pv::Security security_;

  boost::signals2::scoped_connection securityPriceAddedConnection;

public:
  SecurityPriceModel(pv::Security security, QObject* parent = nullptr);

  pv::Date mapToDate(const QModelIndex& index) const noexcept { return dates.at(index.row()); }

  // Overrides

  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override {
    if (parent.isValid())
      return QModelIndex(); // No children
    return createIndex(row, column, nullptr);
  }

  int rowCount(const QModelIndex& parent = QModelIndex()) const override {
    if (parent.isValid())
      return 0; // No children
    return static_cast<int>(dates.size());
  }

  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    if (parent.isValid())
      return 0;
    return 2;
  }

  QModelIndex parent(const QModelIndex&) const override { return QModelIndex(); }

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
signals:
  void dateAdded(pv::Date date);
  void dateRemoved(pv::Date date);
};

} // namespace pvui::models

#endif // PVUI_MODELS_SECURITYPRICEMODEL_H
