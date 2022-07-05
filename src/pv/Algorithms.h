#ifndef PV_ALGORITHMS_ALGORITHMS_H
#define PV_ALGORITHMS_ALGORITHMS_H

#include "pv/DataFile.h"
#include <optional>
#include "pv/Integer64.h"

namespace pv {
namespace algorithms {

i64 cashBalance(const DataFile& dataFile, i64 account, i64 date);

i64 sharesHeld(const DataFile& dataFile, i64 security, i64 date);

i64 sharesHeld(const DataFile& dataFile, i64 security, i64 account, i64 date);

i64 sharesSold(const DataFile& dataFile, i64 security, i64 account);

i64 sharesSold(const DataFile& dataFile, i64 security, i64 account, i64 date);

i64 cashGained(const DataFile& dataFile, i64 security, i64 date);

i64 cashGained(const DataFile& dataFile, i64 security, i64 account, i64 date);

i64 dividendIncome(const DataFile& dataFile, i64 security, i64 date);

i64 dividendIncome(const DataFile& dataFile, i64 security, i64 account, i64 date);

i64 costBasis(const DataFile& dataFile, i64 security, i64 date);

i64 costBasis(const DataFile& dataFile, i64 security, i64 account, i64 date);

i64 totalIncome(const DataFile& dataFile, i64 security, i64 date);

i64 totalIncome(const DataFile& dataFile, i64 security, i64 account, i64 date);

std::optional<i64> sharePrice(const DataFile& dataFile, i64 security, i64 date);

std::optional<i64> unrealizedCashGained(const DataFile& dataFile, i64 security, i64 date);

std::optional<i64> unrealizedCashGained(const DataFile& dataFile, i64 security, i64 account, i64 date);

std::optional<i64> averageBuyPrice(const DataFile& dataFile, i64 security, i64 date); 

std::optional<i64> averageBuyPrice(const DataFile& dataFile, i64 security, i64 account, i64 date); 

std::optional<i64> averageSellPrice(const DataFile& dataFile, i64 security, i64 date); 

std::optional<i64> averageSellPrice(const DataFile& dataFile, i64 security, i64 account, i64 date); 

std::optional<i64> marketValue(const DataFile& dataFile, i64 security, i64 date);

std::optional<i64> marketValue(const DataFile& dataFile, i64 security, i64 account, i64 date);

} // namespace algorithms
} // namespace pv

#endif // PV_ALGORITHMS_ALGORITHMS_H
