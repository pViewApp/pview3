#include "HoldingsModel.h"
#include "DateUtils.h"
#include "ModelUtils.h"
#include "pv/Algorithms.h"
#include "pv/Integer64.h"
#include "pv/Security.h"
#include <QSize>
#include <Qt>
#include <iterator>
#include <memory>
#include <optional>
#include <qnamespace.h>
#include <sqlite3.h>
#include <utility>

constexpr int symbolColumn = 0;
constexpr int nameColumn = 1;
constexpr int sharesHeldColumn = 2;
constexpr int recentQuoteColumn = 3;
constexpr int averageBuyPriceColumn = 4;
constexpr int averageSellPriceColumn = 5;
constexpr int unrealizedGainColumn = 6;
constexpr int unrealizedGainPercentageColumn = 7;
constexpr int realizedGainColumn = 8;
constexpr int dividendIncomeColumn = 9;
constexpr int costBasisColumn = 10;
constexpr int totalIncomeColumn = 11;
constexpr int marketValueColunm = 12;
constexpr int columnCount = 13; // Update whenever new column added

namespace pvui {
namespace models {

HoldingsModel::HoldingsModel(pv::DataFile& dataFile, QObject* parent)
    : QAbstractTableModel(parent), dataFile_(dataFile) {
  changedConnection = dataFile.onChanged([&]() { emit reset(); });

  QObject::connect(this, &HoldingsModel::reset, this, [&] {
    beginResetModel();
    holdings.clear();
    needsRepopulating = true;
    endResetModel();
  });
}

void HoldingsModel::repopulate() {
  beginResetModel();
  holdings.clear();

  auto stmt = dataFile_.query("SELECT Id FROM Securities");
  while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
    pv::i64 security = sqlite3_column_int64(stmt.get(), 0);
    Holding h;
    pv::i64 today = currentEpochDate();
    h.security = security;
    h.symbol = QString::fromStdString(pv::security::symbol(dataFile_, security));
    h.name = QString::fromStdString(pv::security::name(dataFile_, security));
    h.sharesHeld = pv::algorithms::sharesHeld(dataFile_, security, today);
    h.recentQuote = pv::algorithms::sharePrice(dataFile_, security, today);
    h.avgBuyPrice = pv::algorithms::averageBuyPrice(dataFile_, security, today);
    h.avgSellPrice = pv::algorithms::averageSellPrice(dataFile_, security, today);
    h.unrealizedGain = pv::algorithms::unrealizedCashGained(dataFile_, security, today);
    if (!h.unrealizedGain.has_value() || !h.avgBuyPrice.has_value()) {
      h.unrealizedGainPercentage = std::nullopt;
    } else {
      double second = h.avgBuyPrice.value() * h.sharesHeld;
      if (second == 0) {
        h.unrealizedGainPercentage = 0; // Avoid division by zero
      } else {
        h.unrealizedGainPercentage = (h.unrealizedGain.value() * 100) / second;
      }
    }
    h.realizedGain = pv::algorithms::cashGained(dataFile_, security, today);
    h.dividendIncome = pv::algorithms::dividendIncome(dataFile_, security, today);
    h.costBasis = pv::algorithms::costBasis(dataFile_, security, today);
    h.totalIncome = pv::algorithms::totalIncome(dataFile_, security, today);
    h.marketValue = pv::algorithms::marketValue(dataFile_, security, today);

    holdings.push_back(std::move(h));
  }

  needsRepopulating = false;
  endResetModel();
}

bool HoldingsModel::canFetchMore(const QModelIndex& parent) const { return !parent.isValid() && needsRepopulating; }

void HoldingsModel::fetchMore(const QModelIndex&) { repopulate(); }

int HoldingsModel::rowCount(const QModelIndex& index) const { return index.isValid() ? 0 : holdings.size(); }

int HoldingsModel::columnCount(const QModelIndex&) const { return ::columnCount; }

QVariant HoldingsModel::data(const QModelIndex& index, int role) const {
  using modelutils::FormatFlag;
  using modelutils::moneyData;
  using modelutils::numberData;
  using modelutils::percentageData;
  using modelutils::stringData;

  Holding holding = holdings.at(index.row());
  switch (index.column()) {
  case symbolColumn: {
    return stringData(holding.symbol, role);
  }
  case nameColumn: {
    return stringData(holding.name, role);
  }
  case recentQuoteColumn: {
    return holding.recentQuote ? moneyData(*holding.recentQuote, role)
                               : stringData(tr("N/A"), role, FormatFlag::Numeric);
  }
  case averageBuyPriceColumn: {
    return holding.avgBuyPrice ? moneyData(*holding.avgBuyPrice, role)
                               : stringData(tr("N/A"), role, FormatFlag::Numeric);
  }
  case averageSellPriceColumn: {
    return holding.avgSellPrice ? moneyData(*holding.avgSellPrice, role)
                                : stringData(tr("N/A"), role, FormatFlag::Numeric);
  }
  case sharesHeldColumn: {
    return numberData(holding.sharesHeld, role);
  }
  case unrealizedGainColumn: {
    return holding.unrealizedGain ? moneyData(*holding.unrealizedGain, role, FormatFlag::Numeric | FormatFlag::ColorNegative)
                                  : stringData(tr("N/A"), role, FormatFlag::Numeric);
  }
  case unrealizedGainPercentageColumn: {
    return holding.unrealizedGainPercentage
               ? percentageData(*holding.unrealizedGainPercentage, role, FormatFlag::Numeric | FormatFlag::ColorNegative)
               : stringData(tr("N/A"), role, FormatFlag::Numeric);
  }
  case realizedGainColumn: {
    return moneyData(holding.realizedGain, role, FormatFlag::Numeric | FormatFlag::ColorNegative);
  }
  case dividendIncomeColumn: {
    return moneyData(holding.dividendIncome, role);
  }
  case costBasisColumn: {
    return moneyData(holding.costBasis, role);
  }
  case totalIncomeColumn: {
    return moneyData(holding.totalIncome, role, FormatFlag::Numeric | FormatFlag::ColorNegative);
  }
  case marketValueColunm: {
    return holding.marketValue ? moneyData(*holding.marketValue, role)
                               : stringData(tr("N/A"), role, FormatFlag::Numeric);
  }
  default:
    return QVariant();
  }
}

QVariant HoldingsModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
    if (orientation != Qt::Horizontal)
      return QVariant();

    switch (section) {
    case symbolColumn:
      return tr("Symbol");
    case nameColumn:
      return tr("Name");
    case recentQuoteColumn:
      return tr("Recent Quote");
    case averageBuyPriceColumn:
      return tr("Average Buy Price");
    case averageSellPriceColumn:
      return tr("Average Sell Price");
    case sharesHeldColumn:
      return tr("Shares Held");
    case unrealizedGainColumn:
      return tr("Unrealized Gain");
    case unrealizedGainPercentageColumn: {
      return tr("Unrealized Gain (%)");
    }
    case realizedGainColumn:
      return tr("Realized Gain");
    case dividendIncomeColumn:
      return tr("Dividend Income");
    case costBasisColumn:
      return tr("Cost Basis");
    case totalIncomeColumn:
      return tr("Total Income");
    case marketValueColunm:
      return tr("Market Value");
    default:
      return QVariant();
    }
  } else {
    return QVariant();
  }
}

} // namespace models
} // namespace pvui
