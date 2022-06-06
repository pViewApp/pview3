#ifndef PV_ALGORITHMS_ALGORITHMS_H
#define PV_ALGORITHMS_ALGORITHMS_H

#include "Decimal.h"
#include "pv/DataFile.h"
#include <optional>

namespace pv {
namespace algorithms {

Security* security(const Transaction* transaction);

Decimal cashBalance(const Transaction* transaction);

Decimal cashBalance(const Account& account, Date date = dates::today());

Decimal sharesHeld(const Security& security, const Account& account, Date date = dates::today());

Decimal sharesHeld(const Security& security, Date date = dates::today());

Decimal cashSpent(const Security& security, const Account& account, Date date = dates::today());

Decimal cashSpent(const Security& security, Date date = dates::today());

Decimal cashEarned(const Security& security, const Account& account, Date date = dates::today());

Decimal cashEarned(const Security& security, Date date = dates::today());

Decimal cashGained(const Security& security, const Account& account, Date date = dates::today());

Decimal cashGained(const Security& security, Date date = dates::today());

Decimal dividendIncome(const Security& security, const Account& account, Date date = dates::today());

Decimal dividendIncome(const Security& security, Date date = dates::today());

Decimal costBasis(const Security& security, const Account& account, Date date = dates::today());

Decimal costBasis(const Security& security, Date date = dates::today());

std::optional<Decimal> unrealizedCashGained(const Security& security, const Account& account,
                                            Date date = dates::today());

std::optional<Decimal> unrealizedCashGained(const Security& security, Date date = dates::today());

std::optional<Decimal> sharePrice(const Security& security, Date date = dates::today());

std::optional<Decimal> marketValue(const Security& security, const Account& account, Date date = dates::today());

std::optional<Decimal> marketValue(const Security& security, Date date = dates::today());

std::optional<Decimal> averageBuyPrice(const Security& security, Date date = dates::today());

std::optional<Decimal> averageSellPrice(const Security& security, Date date = dates::today());

std::optional<Decimal> averageBuyPrice(const Security& security, const Account& account, Date date = dates::today());

std::optional<Decimal> averageSellPrice(const Security& security, const Account& account, Date date = dates::today());

std::optional<Decimal> unrealizedGainRelative(const Security& security, const Account& account,
                                              Date date = dates::today());
std::optional<Decimal> unrealizedGainRelative(const Security& security, Date date = dates::today());

Decimal totalIncome(const Security& security, const Account& account, Date date = dates::today());

Decimal totalIncome(const Security& security, Date date = dates::today());

} // namespace algorithms
} // namespace pv

#endif // PV_ALGORITHMS_ALGORITHMS_H
