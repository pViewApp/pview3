#ifndef PV_ALGORITHMS_ALGORITHMS_H
#define PV_ALGORITHMS_ALGORITHMS_H

#include "pv/DataFile.h"
#include <optional>
#include "pv/Integer64.h"

namespace pv {
namespace algorithms {

i64 cashBalance(DataFile& dataFile, i64 account, i64 date);

i64 sharesHeld(DataFile& dataFile, i64 security, i64 date);

i64 sharesHeld(DataFile& dataFile, i64 security, i64 account, i64 date);

i64 sharesSold(DataFile& dataFile, i64 security, i64 account);

i64 sharesSold(DataFile& dataFile, i64 security, i64 account, i64 date);

i64 cashGained(DataFile& dataFile, i64 security, i64 date);

i64 cashGained(DataFile& dataFile, i64 security, i64 account, i64 date);

i64 dividendIncome(DataFile& dataFile, i64 security, i64 date);

i64 dividendIncome(DataFile& dataFile, i64 security, i64 account, i64 date);

i64 interestIncome(DataFile& dataFile, i64 security, i64 date);

i64 interestIncome(DataFile& dataFile, i64 security, i64 account, i64 date);

i64 costBasis(DataFile& dataFile, i64 security, i64 date);

i64 costBasis(DataFile& dataFile, i64 security, i64 account, i64 date);

i64 totalIncome(DataFile& dataFile, i64 security, i64 date);

i64 totalIncome(DataFile& dataFile, i64 security, i64 account, i64 date);

std::optional<i64> sharePrice(DataFile& dataFile, i64 security, i64 date);

std::optional<i64> unrealizedCashGained(DataFile& dataFile, i64 security, i64 date);

std::optional<i64> unrealizedCashGained(DataFile& dataFile, i64 security, i64 account, i64 date);

std::optional<i64> averageBuyPrice(DataFile& dataFile, i64 security, i64 date);

std::optional<i64> averageBuyPrice(DataFile& dataFile, i64 security, i64 account, i64 date);

std::optional<i64> averageSellPrice(DataFile& dataFile, i64 security, i64 date);

std::optional<i64> averageSellPrice(DataFile& dataFile, i64 security, i64 account, i64 date);

std::optional<i64> marketValue(DataFile& dataFile, i64 security, i64 date);

std::optional<i64> marketValue(DataFile& dataFile, i64 security, i64 account, i64 date);

} // namespace algorithms
} // namespace pv

#endif // PV_ALGORITHMS_ALGORITHMS_H
