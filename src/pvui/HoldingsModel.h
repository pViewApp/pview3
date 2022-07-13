#ifndef PVUI_MODELS_HOLDINGSMODEL_H
#define PVUI_MODELS_HOLDINGSMODEL_H

#include "pv/DataFile.h"
#include "pv/Integer64.h"
#include "pv/Signals.h"
#include <QAbstractTableModel>
#include <qabstractitemmodel.h>
#include <unordered_map>

class sqlite3_stmt;
namespace pvui {
namespace models {

class HoldingsModel : public QAbstractTableModel {
  Q_OBJECT
private:
  bool needsRepopulating = true;

  pv::DataFile& dataFile_;

  struct Holding {
    pv::i64 security;
    QString symbol;
    QString name;
    pv::i64 sharesHeld;
    std::optional<pv::i64> recentQuote;
    std::optional<pv::i64> avgBuyPrice;
    std::optional<pv::i64> avgSellPrice;
    std::optional<pv::i64> unrealizedGain;
    std::optional<double> unrealizedGainPercentage;
    pv::i64 realizedGain;
    pv::i64 dividendIncome;
    pv::i64 interestIncome;
    pv::i64 costBasis;
    pv::i64 totalIncome;
    std::optional<pv::i64> marketValue;
  };

  std::vector<Holding> holdings;

  // Connections
  pv::ScopedConnection changedConnection;
  pv::ScopedConnection resetConnection;

  void repopulate();
public:
  explicit HoldingsModel(pv::DataFile& dataFile, QObject* parent = nullptr);

  int rowCount(const QModelIndex& index = QModelIndex()) const override;
  int columnCount(const QModelIndex& = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  bool canFetchMore(const QModelIndex& parent) const override;

  void fetchMore(const QModelIndex&) override;
signals:
  void reset();

  // QAbstractItemModel interface
};

} // namespace models
} // namespace pvui

#endif // PVUI_MODELS_HOLDINGSMODEL_H
