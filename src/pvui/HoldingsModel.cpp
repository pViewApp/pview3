#include "HoldingsModel.h"
#include "ModelUtils.h"
#include "pv/Algorithms.h"
#include "pv/Integer64.h"
#include <iterator>
#include <memory>
#include <optional>
#include <Qt>
#include <qnamespace.h>
#include <sqlite3.h>
#include "pv/Security.h"
#include <QSize>
#include <utility>
#include "DateUtils.h"

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
  repopulate();

  changedConnection =
      dataFile.onChanged([&]() { emit reset(); });

  QObject::connect(this, &HoldingsModel::reset, this, [&]{ beginResetModel(); repopulate(); endResetModel(); });
}

void HoldingsModel::repopulate() {
  securities.clear();

  auto stmt = dataFile_.query("SELECT Id FROM Securities");
  while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
    securities.push_back(sqlite3_column_int64(stmt.get(), 0));
  }
}


void HoldingsModel::update(pv::i64 security) {
  auto rowNumber = static_cast<int>(std::find(securities.begin(), securities.end(), security) - securities.begin());

  auto firstIndex = index(rowNumber, 0);
  auto lastIndex = index(rowNumber, ::columnCount);

  emit dataChanged(firstIndex, lastIndex);
}

int HoldingsModel::rowCount(const QModelIndex&) const { return static_cast<int>(securities.size()); }

int HoldingsModel::columnCount(const QModelIndex&) const { return ::columnCount; }

QVariant HoldingsModel::data(const QModelIndex& index, int role) const {
  using modelutils::FormatFlag;
  using modelutils::numberData;
  using modelutils::percentageData;
  using modelutils::moneyData;

  if (role == Qt::TextAlignmentRole) {
    int col = index.column();
    if (col == symbolColumn || col == nameColumn) {
      return QVariant(); // Use default alignment for text column
    } else {
      return QVariant(Qt::AlignRight | Qt::AlignVCenter); // right-align numbers
    }
  }
  if (role != Qt::DisplayRole && role != Qt::AccessibleTextRole && role != Qt::ForegroundRole)
    return QVariant();

  pv::i64 security = securities.at(index.row());
  switch (index.column()) {
  case symbolColumn: {
    return QString::fromStdString(pv::security::symbol(dataFile_, security));
  }
  case nameColumn: {
    return QString::fromStdString(pv::security::name(dataFile_, security));
  }
  case recentQuoteColumn: {
    std::optional<pv::i64> recentQuote = pv::algorithms::sharePrice(dataFile_, security, currentEpochDate());
    return recentQuote ? moneyData(*recentQuote, role) : tr("N/A");
  }
  case averageBuyPriceColumn: {
    std::optional<pv::i64> averageBuyPrice = pv::algorithms::averageBuyPrice(dataFile_, security, currentEpochDate());
    return averageBuyPrice.has_value() ? moneyData(*averageBuyPrice, role) : QVariant(tr("N/A"));
  }
  case averageSellPriceColumn: {
    std::optional<pv::i64> averageSellPrice = pv::algorithms::averageSellPrice(dataFile_, security, currentEpochDate());
    return averageSellPrice.has_value() ? moneyData(*averageSellPrice, role) : QVariant(tr("N/A"));
  }
  case sharesHeldColumn: {
    return numberData(pv::algorithms::sharesHeld(dataFile_, security, currentEpochDate()), role);
  }
  case unrealizedGainColumn: {
    return moneyData(pv::algorithms::unrealizedCashGained(dataFile_, security, currentEpochDate()).value_or(0), role,
                           FormatFlag::COLOR_NEGATIVE);
  }
  case unrealizedGainPercentageColumn: {
    std::optional<pv::i64> averageBuyPrice = pv::algorithms::averageBuyPrice(dataFile_, security, currentEpochDate());
    std::optional<pv::i64> sharesHeld = pv::algorithms::sharesHeld(dataFile_, security, currentEpochDate());
    std::optional<pv::i64> unrealizedCashGained = pv::algorithms::unrealizedCashGained(dataFile_, security, currentEpochDate());
    
    pv::i64 second = sharesHeld.value_or(0) * averageBuyPrice.value_or(0);

    if (second == 0) {
      return percentageData(0, role, FormatFlag::COLOR_NEGATIVE);
    } else {
      return percentageData((unrealizedCashGained.value_or(0) * 100) / static_cast<double>(second), role, FormatFlag::COLOR_NEGATIVE);
    }
  }
  case realizedGainColumn: {
    return moneyData(pv::algorithms::cashGained(dataFile_, security, currentEpochDate()), role, FormatFlag::COLOR_NEGATIVE);
  }
  case dividendIncomeColumn: {
    return moneyData(pv::algorithms::dividendIncome(dataFile_, security, currentEpochDate()), role);
  }
  case costBasisColumn: {
    return moneyData(pv::algorithms::costBasis(dataFile_, security, currentEpochDate()), role);
  }
  case totalIncomeColumn: {
    return moneyData(pv::algorithms::totalIncome(dataFile_, security, currentEpochDate()), role, FormatFlag::COLOR_NEGATIVE);
  }
  case marketValueColunm: {
    std::optional<pv::i64> marketValue = pv::algorithms::marketValue(dataFile_, security, currentEpochDate());
    if (!marketValue.has_value()) {
      return tr("N/A");
    }
    return moneyData(*marketValue, role);
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
