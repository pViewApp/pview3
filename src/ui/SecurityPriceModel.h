#ifndef PVUI_MODELS_SECURITYPRICEMODEL_H
#define PVUI_MODELS_SECURITYPRICEMODEL_H

#include "DataFile.h"
#include "Types.h"
#include <QAbstractItemModel>
#include <set>
#include <vector>

namespace pvui::models {
class SecurityPriceModel : public QAbstractItemModel {
  Q_OBJECT
private:
  std::vector<pv::Date> dates;
  pv::SecurityPtr security_;

public:
  SecurityPriceModel(pv::SecurityPtr security, QObject* parent = nullptr);

  inline QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override {
    if (parent.isValid()) return QModelIndex(); // No children
    return createIndex(row, column, nullptr);
  }

  inline int rowCount(const QModelIndex& parent = QModelIndex()) const override {
    if (parent.isValid())
      return 0; // No children
    return static_cast<int>(dates.size());
  }

  inline int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    if (parent.isValid())
      return 0;
    return 2;
  }

  inline QModelIndex parent(const QModelIndex&) const override { return QModelIndex(); }

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
signals:
  void dateAdded(pv::Date date);
  void dateRemoved(pv::Date date);

  void beforeDateAdded();
  void afterDateAdded();
  void beforeDateRemoved();
  void afterDateRemoved();
};
} // namespace pvui::models

#endif // PVUI_MODELS_SECURITYPRICEMODEL_H
