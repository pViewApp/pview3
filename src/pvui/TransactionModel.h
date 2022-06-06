#ifndef PVUI_MODELS_TRANSACTIONMODEL_H
#define PVUI_MODELS_TRANSACTIONMODEL_H

#include "ActionData.h"
#include "pv/Account.h"
#include "pv/Signals.h"
#include "pv/Transaction.h"
#include <QAbstractTableModel>
#include <QDate>
#include <QObject>
#include <optional>
#include <vector>

namespace pvui::models {
class TransactionModel : public QAbstractTableModel {
  Q_OBJECT
private:
  /// \internal
  /// \brief A structure containing the displayable traits of a transaction.
  struct DisplayData {
    pv::Date date;
    const pvui::ActionData* action;
    pv::Security* security;
    pv::Decimal numberOfShares; // can be NaN
    pv::Decimal sharePrice;     // can be NaN
    pv::Decimal commission;     // can be NaN
    pv::Decimal totalAmount;    // can be NaN
  };

  pv::Account& account_;
  pv::DataFile& dataFile_;
  pv::ScopedConnection transactionAddedConnection;
  pv::ScopedConnection transactionChangedConnection;
  pv::ScopedConnection transactionRemovedConnection;

  /// \internal
  /// \note The indexes in here should match with those of account_.transactions(). That is, for any number *n*
  /// \c transactions[n] should represent \c account_.transactions[n].
  std::vector<DisplayData> transactions;

  DisplayData createDisplayData(const pv::Transaction& transaction) noexcept;

public:
  TransactionModel(pv::DataFile& dataFile, pv::Account& account, QObject* parent = nullptr);

  int rowCount(const QModelIndex& = QModelIndex()) const override { return static_cast<int>(transactions.size()); }

  int columnCount(const QModelIndex&) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
signals:
  void transactionAdded(std::size_t index, const pv::Transaction& transaction);
  void transactionChanged(std::size_t index, const pv::Transaction& newTransaction);
  void transactionRemoved(std::size_t index);
};
} // namespace pvui::models

#endif // PVUI_MODELS_TRANSACTIONMODEL_H
