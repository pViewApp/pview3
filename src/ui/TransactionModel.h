#ifndef PVUI_MODELS_TRANSACTIONMODEL_H
#define PVUI_MODELS_TRANSACTIONMODEL_H

#include "DataFile.h"
#include <QAbstractItemModel>
#include <QObject>

namespace pvui::models {
class TransactionModel : public QAbstractItemModel {
  Q_OBJECT
private:
  const pv::AccountPtr account_;

public:
  TransactionModel(const pv::AccountPtr account, QObject* parent = nullptr);

  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override {
    if (parent.isValid())
      return QModelIndex(); // Only top-level parents are
    return createIndex(
        row, column,
        row > rowCount() - 1 ? nullptr : account_->transactions().at(row).get()); // Use nullptr if this row is greater
                                                                                  // than the number of transactions
  }

  int rowCount(const QModelIndex& parent = QModelIndex()) const override {
    if (parent.isValid())
      return 0; // Only top-level parents are allowed
    return static_cast<int>(account_->transactions().size());
  }

  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    if (parent.isValid())
      return 0; // Only top-level parents are allowed
    return 7;   // The columns are Date, Action, Security, Number of Shares, Share
                // Price, Commission, Total Amount
  }

  QModelIndex parent(const QModelIndex&) const override {
    return QModelIndex(); // There cannot be any parents since there will never
                          // be children
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
signals:
  void beforeTransactionAdded(const QModelIndex& index, int first, int last);
  void afterTransactionAdded();
};
} // namespace pvui::models

#endif // PVUI_MODELS_TRANSACTIONMODEL_H
