#ifndef PVUI_MODELS_TRANSACTIONMODEL_H
#define PVUI_MODELS_TRANSACTIONMODEL_H

#include "ActionData.h"
#include "pv/DataFile.h"
#include "pv/Integer64.h"
#include "pv/Signals.h"
#include "pv/Transaction.h"
#include <QAbstractTableModel>
#include <QDate>
#include <QObject>
#include <optional>
#include <qabstractitemmodel.h>
#include <vector>

namespace pvui::models {
class TransactionModel : public QAbstractTableModel {
  Q_OBJECT
private:
  pv::DataFile& dataFile;
  pv::i64 account;

  std::vector<pv::i64> transactions;

  pv::ScopedConnection transactionAddedConnection;
  pv::ScopedConnection transactionRemovedConnection;
  pv::ScopedConnection transactionUpdatedConnection;
  pv::ScopedConnection resetConnection;

  void repopulate();
public:
  TransactionModel(pv::DataFile& dataFile, pv::i64 account, QObject* parent = nullptr);

  int indexOfTransaction(pv::i64 transaction);
  pv::i64 transactionOfIndex(int rowIndex);

  int rowCount(const QModelIndex& = QModelIndex()) const override { return static_cast<int>(transactions.size()); }

  int columnCount(const QModelIndex&) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
private slots:
  void handleTransactionAdded(pv::i64 id);
  void handleTransactionUpdated(pv::i64 id);
  void handleTransactionRemoved(pv::i64 id);
  void handleReset();
signals:
  void transactionAdded(pv::i64 id);
  void transactionUpdated(pv::i64 id);
  void transactionRemoved(pv::i64 id);
  void reset();
};
} // namespace pvui::models

#endif // PVUI_MODELS_TRANSACTIONMODEL_H
