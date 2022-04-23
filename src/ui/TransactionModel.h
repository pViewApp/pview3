#ifndef PVUI_MODELS_TRANSACTIONMODEL_H
#define PVUI_MODELS_TRANSACTIONMODEL_H

#include "DataFile.h"
#include <QAbstractTableModel>
#include <QObject>
#include <boost/signals2.hpp>
#include <vector>

namespace pvui::models {
class TransactionModel : public QAbstractTableModel {
  Q_OBJECT
private:
  const pv::AccountPtr account_;
  boost::signals2::scoped_connection transactionAddedConnection;

  std::vector<pv::TransactionPtr> transactions;

public:
  TransactionModel(const pv::AccountPtr account, QObject* parent = nullptr);

  int rowCount(const QModelIndex& = QModelIndex()) const override { return static_cast<int>(transactions.size()); }

  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    if (parent.isValid())
      return 0; // Only top-level parents are allowed
    return 7;   // The columns are Date, Action, Security, Number of Shares, Share
                // Price, Commission, Total Amount
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
signals:
  void transactionAdded(pv::TransactionPtr transaction);
};
} // namespace pvui::models

#endif // PVUI_MODELS_TRANSACTIONMODEL_H
