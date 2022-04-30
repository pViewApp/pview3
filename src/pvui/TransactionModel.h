#ifndef PVUI_MODELS_TRANSACTIONMODEL_H
#define PVUI_MODELS_TRANSACTIONMODEL_H

#include "pv/Account.h"
#include "pv/Transaction.h"
#include <QAbstractTableModel>
#include <QObject>
#include <boost/signals2.hpp>
#include <vector>

namespace pvui::models {
class TransactionModel : public QAbstractTableModel {
  Q_OBJECT
private:
  const pv::Account account_;
  boost::signals2::scoped_connection transactionAddedConnection;
  boost::signals2::scoped_connection transactionChangedConnection;
  boost::signals2::scoped_connection transactionRemovedConnection;

  std::vector<pv::Transaction> transactions;

public:
  TransactionModel(const pv::Account account, QObject* parent = nullptr);

  int rowCount(const QModelIndex& = QModelIndex()) const override { return static_cast<int>(transactions.size()); }

  int columnCount(const QModelIndex&) const override {
    return 7; // The columns are Date, Action, Security, Number of Shares, Share
              // Price, Commission, Total Amount
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role) override;

  QModelIndex mapToIndex(const pv::Transaction& transaction) const noexcept;
  std::optional<pv::Transaction> mapFromIndex(const QModelIndex& index) noexcept;
signals:
  void transactionAdded(const pv::Transaction& transaction);
  void transactionChanged(const pv::Transaction& transaction);
  void transactionRemoved(const pv::Transaction& transaction);
};
} // namespace pvui::models

#endif // PVUI_MODELS_TRANSACTIONMODEL_H
