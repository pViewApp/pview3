#include "HoldingsModel.h"
#include "ModelUtils.h"
#include "pv/Algorithms.h"
#include <QSize>
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

void HoldingsModel::update(const pv::Security& security, int column) {
  int rowIndex = std::find(securities.cbegin(), securities.cend(), security) - securities.cbegin();
  const auto modelIndex = index(rowIndex, column);

  emit dataChanged(modelIndex, modelIndex);
}

void HoldingsModel::createListeners(const pv::Security& security) {
  // Name
  securityChangeConnections.insert(
      {security, security.nameChanged().connect([=](std::string, std::string) { update(security, nameColumn); })});
}

void HoldingsModel::removeListeners(const pv::Security& security) { securityChangeConnections.erase(security); }

HoldingsModel::HoldingsModel(const pv::DataFile& dataFile, QObject* parent)
    : QAbstractTableModel(parent), dataFile_(dataFile) {
  securityAddedConnection =
      dataFile.securityAdded().connect([&](const pv::Security& security) { emit securityAdded(security); });

  securityRemovedConnection =
      dataFile.securityRemoved().connect([&](const pv::Security& security) { emit securityRemoved(security); });

  QObject::connect(this, &HoldingsModel::securityAdded, this, [&](const pv::Security& security) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    securities.push_back(security);
    endInsertRows();
    createListeners(security);
  });

  QObject::connect(this, &HoldingsModel::securityRemoved, this, [&](const pv::Security& security) {
    auto iter = std::find(securities.cbegin(), securities.cend(), security);
    auto rowIndex = iter - securities.cbegin();

    beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
    securities.erase(iter);
    endRemoveRows();
  });
}

int HoldingsModel::rowCount(const QModelIndex&) const { return static_cast<int>(securities.size()); }

int HoldingsModel::columnCount(const QModelIndex&) const { return ::columnCount; }

QVariant HoldingsModel::data(const QModelIndex& index, int role) const {
  if (role != Qt::DisplayRole && role != Qt::AccessibleTextRole)
    return QVariant();

  pv::Security security = securities.at(index.row());
  switch (index.column()) {
  case symbolColumn: {
    return QString::fromStdString(security.symbol());
  }
  case nameColumn: {
    return QString::fromStdString(security.name());
  }
  case recentQuoteColumn: {
    std::optional<pv::Decimal> recentQuote = pv::algorithms::sharePrice(security);

    if (recentQuote.has_value()) {
      return QString("$%1").arg(QString::fromStdString(recentQuote->str()));
    } else {
      return tr("N/A");
    }
  }
  case averageBuyPriceColumn: {
    std::optional<pv::Decimal> averageBuyPrice = pv::algorithms::averageBuyPrice(security);
    return averageBuyPrice.has_value() ? util::moneyData(*averageBuyPrice, role) : QVariant(tr("N/A"));
  }
  case averageSellPriceColumn: {
    std::optional<pv::Decimal> averageSellPrice = pv::algorithms::averageSellPrice(security);
    return averageSellPrice.has_value() ? util::moneyData(*averageSellPrice, role) : QVariant(tr("N/A"));
  }
  case sharesHeldColumn: {
    return static_cast<double>(pv::algorithms::sharesHeld(security));
  }
  case unrealizedGainColumn: {
    return util::moneyData(pv::algorithms::unrealizedCashGained(security).value_or(0), role);
  }
  case unrealizedGainPercentageColumn: {
    return util::percentageData(pv::algorithms::unrealizedGainRelative(security).value_or(0) * 100, role);
  }
  case realizedGainColumn: {
    return util::moneyData(pv::algorithms::cashGained(security), role);
  }
  case dividendIncomeColumn: {
    return util::moneyData(pv::algorithms::dividendIncome(security), role);
  }
  case costBasisColumn: {
    return util::moneyData(pv::algorithms::costBasis(security), role);
  }
  case totalIncomeColumn: {
    return util::moneyData(pv::algorithms::totalIncome(security), role);
  }
  case marketValueColunm: {
    std::optional<pv::Decimal> marketValue = pv::algorithms::marketValue(security);
    if (!marketValue.has_value()) {
      return tr("N/A");
    }
    return util::moneyData(*marketValue, role);
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
