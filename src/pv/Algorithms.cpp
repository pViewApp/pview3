#include "Algorithms.h"
#include "pv/Transaction.h"
#include <algorithm>
#include <execution>
#include <valarray>
#include <vector>

namespace {

template <typename FwdIt> std::optional<pv::Decimal> weightedAverage(FwdIt begin, FwdIt end) {
  // https://stackoverflow.com/a/57769088
  using Value = std::tuple<pv::Decimal, pv::Decimal>; // First decimal is the value, second is the weight
  auto [values, weights] =
      std::accumulate(begin, end, std::make_tuple<pv::Decimal, pv::Decimal>(0, 0), [](const Value& a, const Value& b) {
        auto [valueA, weightA] = a;
        auto [valueB, weightB] = b;
        return std::make_tuple<pv::Decimal, pv::Decimal>(valueB * weightB + valueA, weightB + weightA);
      });

  if (weights == 0) {
    return std::nullopt;
  }
  return values / weights;
}
} // namespace

namespace pv {
namespace algorithms {

Decimal cashBalance(const Account& account, Date date) {
  std::vector<const Transaction*> transactions;
  std::copy_if(account.transactions().cbegin(), account.transactions().cend(), std::back_inserter(transactions),
               [&date](const Transaction* t) { return t->date <= date; });

  if (transactions.size() == 0)
    return 0;

  std::valarray<Decimal> cashBalances(transactions.size());
  std::transform(transactions.cbegin(), transactions.cend(), std::begin(cashBalances),
                 [](const Transaction* transaction) { return cashBalance(transaction); });

  return cashBalances.sum();
}

Decimal sharesHeld(const Security& security, const Account& account, Date date) {
  std::vector<pv::Decimal> numberOfSharesPerTransaction;
  for (const auto* t : account.transactions()) {

    if (t->date > date || algorithms::security(t) != &security)
      continue;

    const auto action = t->action();
    if (action == Action::BUY) {
      numberOfSharesPerTransaction.push_back(static_cast<const BuyTransaction*>(t)->numberOfShares);
    } else if (action == Action::SELL) {
      numberOfSharesPerTransaction.push_back(static_cast<const SellTransaction*>(t)->numberOfShares);
    }
  }

  return std::reduce(numberOfSharesPerTransaction.cbegin(), numberOfSharesPerTransaction.cend());
}

Decimal sharesHeld(const Security& security, Date date) {
  if (security.dataFile().accounts().size() == 0) {
    return 0;
  }

  std::valarray<Decimal> perAccount(security.dataFile().accounts().size());
  std::transform(security.dataFile().accounts().cbegin(), security.dataFile().accounts().cend(), std::begin(perAccount),
                 [&security, &date](const Account* account) { return sharesHeld(security, *account, date); });

  return perAccount.sum();
}

std::optional<Decimal> sharePrice(const Security& security, Date date) {
  const std::map<Date, Decimal>& prices = security.prices();
  // Get the price after date
  auto iter = prices.upper_bound(date);

  // If iter is equal to the beginning, there are no prices less than or equal to date
  if (iter == prices.cbegin()) {
    return std::nullopt;
  }

  --iter;

  return iter->second;
}

std::optional<Decimal> marketValue(const Security& security, Date date) {
  const auto sharePrice_ = sharePrice(security, date);
  if (!sharePrice_.has_value())
    return std::nullopt;
  return sharesHeld(security, date) * sharePrice_.value();
}

Decimal cashSpent(const Security& security, Date date) {
  if (security.dataFile().accounts().size() == 0) {
    return 0;
  }

  std::valarray<Decimal> perAccount(security.dataFile().accounts().size());
  std::transform(security.dataFile().accounts().cbegin(), security.dataFile().accounts().cend(), std::begin(perAccount),
                 [&security, &date](const Account* account) { return cashSpent(security, *account, date); });

  return perAccount.sum();
}

Decimal cashSpent(const Security& security, const Account& account, Date date) {
  std::vector<pv::Decimal> cashSpentPerTransaction;
  for (const auto& t : account.transactions()) {
    Decimal cashBalance = algorithms::cashBalance(t);
    if (algorithms::security(t) == &security && cashBalance < 0 && t->date <= date) {
      cashSpentPerTransaction.push_back(cashBalance);
    }
  }

  return std::reduce(cashSpentPerTransaction.cbegin(), cashSpentPerTransaction.cend());
}

Decimal cashEarned(const Security& security, const Account& account, Date date) {
  std::vector<pv::Decimal> cashEarnedPerTransaction;
  for (const auto& t : account.transactions()) {
    Decimal cashBalance = algorithms::cashBalance(t);
    if (algorithms::security(t) == &security && cashBalance < 0 && t->date <= date) {
      cashEarnedPerTransaction.push_back(cashBalance);
    }
  }

  return std::reduce(cashEarnedPerTransaction.cbegin(), cashEarnedPerTransaction.cend());
}

Decimal cashEarned(const Security& security, Date date) {
  if (security.dataFile().accounts().size() == 0) {
    return 0;
  }

  std::valarray<Decimal> perAccount(security.dataFile().accounts().size());
  std::transform(security.dataFile().accounts().cbegin(), security.dataFile().accounts().cend(), std::begin(perAccount),
                 [&security, &date](const Account* account) { return cashEarned(security, *account, date); });

  return perAccount.sum();
}

Decimal cashGained(const Security& security, const Account& account, Date date) {
  return cashEarned(security, account, date) - cashSpent(security, account, date);
}

Decimal cashGained(const Security& security, Date date) {
  return cashEarned(security, date) - cashSpent(security, date);
}

std::optional<Decimal> averageBuyPrice(const Security& security, Date date) {
  using Buy = std::tuple<Decimal, Decimal>; // tuple with the first being the share price and the second being the
                                            // number of shares
  std::vector<Buy> buys;
  for (const auto& account : security.dataFile().accounts()) {
      for (const auto* transaction : account->transactions()) {
      if (transaction->action() != Action::BUY)
        continue;

      auto t = static_cast<const BuyTransaction*>(transaction);
      if (t->date <= date && t->security == &security) {
        buys.push_back({t->sharePrice, t->numberOfShares});
      }
    }
  }

  return weightedAverage(buys.begin(), buys.end());
}

std::optional<Decimal> averageSellPrice(const Security& security, Date date) {
  using Sell = std::tuple<Decimal, Decimal>; // tuple with the first being the share price and the second being the
                                             // number of shares
  std::vector<Sell> sells;
  for (const auto& account : security.dataFile().accounts()) {
      for (const auto* transaction : account->transactions()) {
      if (transaction->action() != Action::SELL)
        continue;

      auto t = static_cast<const BuyTransaction*>(transaction);
      if (t->date <= date && t->security == &security) {
        sells.push_back({t->sharePrice, t->numberOfShares});
      }
    }
  }

  return weightedAverage(sells.begin(), sells.end());
}

std::optional<Decimal> averageBuyPrice(const Security& security, const Account& account, Date date) {
  using Buy = std::tuple<Decimal, Decimal>; // tuple with the first being the share price and the second being the
                                            // number of shares
  std::vector<Buy> buys;
  for (const auto* transaction : account.transactions()) {
    if (transaction->action() != Action::BUY)
      continue;

    auto t = static_cast<const BuyTransaction*>(transaction);
    if (t->date <= date && t->security == &security) {
      buys.push_back({t->sharePrice, t->numberOfShares});
    }
  }

  return weightedAverage(buys.begin(), buys.end());
}

std::optional<Decimal> averageSellPrice(const Security& security, const Account& account, Date date) {
  using Sell = std::tuple<Decimal, Decimal>; // tuple with the first being the share price and the second being the
                                             // number of shares
  std::vector<Sell> sells;
  for (const auto* transaction : account.transactions()) {
    if (transaction->action() != Action::SELL)
      continue;

    auto t = static_cast<const BuyTransaction*>(transaction);
    if (t->date <= date && t->security == &security) {
      sells.push_back({t->sharePrice, t->numberOfShares});
    }
  }

  return weightedAverage(sells.begin(), sells.end());
}

std::optional<Decimal> unrealizedGainRelative(const Security& security, const Account& account, Date date) {
  const std::optional<Decimal> averageBuyPrice_ = averageBuyPrice(security, date);
  const std::optional<Decimal> sharesHeld_ = sharesHeld(security, account, date);
  const std::optional<Decimal> unrealizedCashGained_ = unrealizedCashGained(security, account, date);
  if (!averageBuyPrice_.has_value() || !sharesHeld_.has_value() || !unrealizedCashGained_.has_value()) {
    return std::nullopt;
  }

  return unrealizedCashGained_.value() / (averageBuyPrice_.value() * sharesHeld_.value());
}

std::optional<Decimal> marketValue(const Security& security, const Account& account, Date date) {
  const auto sharePrice_ = sharePrice(security, date);
  if (!sharePrice_.has_value())
    return std::nullopt;
  return sharesHeld(security, account, date) * sharePrice_.value();
}

std::optional<Decimal> unrealizedGainRelative(const Security& security, Date date) {
  const std::optional<Decimal> averageBuyPrice_ = averageBuyPrice(security, date);
  const std::optional<Decimal> sharesHeld_ = sharesHeld(security, date);
  const std::optional<Decimal> unrealizedCashGained_ = unrealizedCashGained(security, date);
  if (!averageBuyPrice_.has_value() || !sharesHeld_.has_value() || !unrealizedCashGained_.has_value()) {
    return std::nullopt;
  }

  return unrealizedCashGained_.value() / (averageBuyPrice_.value() * sharesHeld_.value());
}

std::optional<Decimal> unrealizedCashGained(const Security& security, const Account& account, Date date) {
  const std::optional<Decimal> sharePrice_ = sharePrice(security, date);
  const std::optional<Decimal> averageBuyPrice_ = averageBuyPrice(security, account, date);
  if (!sharePrice_.has_value() || !averageBuyPrice_.has_value()) {
    return std::nullopt;
  }

  return sharesHeld(security, account, date) * (sharePrice_.value() - averageBuyPrice_.value());
}

std::optional<Decimal> unrealizedCashGained(const Security& security, Date date) {
  const std::optional<Decimal> sharePrice_ = sharePrice(security, date);
  const std::optional<Decimal> averageBuyPrice_ = averageBuyPrice(security, date);
  if (!sharePrice_.has_value() || !averageBuyPrice_.has_value()) {
    return std::nullopt;
  }

  return sharesHeld(security, date) * (sharePrice_.value() - averageBuyPrice_.value());
}

Decimal totalIncome(const Security& security, const Account& account, Date date) {
  return unrealizedCashGained(security, account, date).value_or(0) + cashGained(security, account, date) +
         dividendIncome(security, account, date);
}

Decimal dividendIncome(const Security& security, const Account& account, Date date) {
  std::vector<pv::Decimal> dividendsPerTransaction;
  for (const auto& t : account.transactions()) {
    auto tSecurity = algorithms::security(t);
    if (&security == tSecurity && t->date <= date && t->action() == Action::DIVIDEND) {
      dividendsPerTransaction.push_back(static_cast<const DividendTransaction*>(t)->amount);
    }
  }

  return std::reduce(dividendsPerTransaction.cbegin(), dividendsPerTransaction.cend());
}

Decimal dividendIncome(const Security& security, Date date) {
  pv::Decimal total = 0;
  for (const auto* account : security.dataFile().accounts()) {
    total += dividendIncome(security, *account, date);
  }

  return total;
}

Decimal totalIncome(const Security& security, Date date) {
  return unrealizedCashGained(security, date).value_or(0) + cashGained(security, date) + dividendIncome(security, date);
}

Decimal costBasis(const Security& security, const Account& account, Date date) {
  return sharesHeld(security, account, date) * averageBuyPrice(security, account, date).value_or(0);
}

Decimal costBasis(const Security& security, Date date) {
  return sharesHeld(security, date) * averageBuyPrice(security, date).value_or(0);
}

Decimal cashBalance(const Transaction* transaction) {
  switch (transaction->action()) {
  case Action::BUY: {
    auto* t = static_cast<const BuyTransaction*>(transaction);
    return -((t->numberOfShares * t->sharePrice) + t->commission);
  }
  case Action::SELL: {
    auto* t = static_cast<const SellTransaction*>(transaction);
    return (t->numberOfShares * t->sharePrice) - t->commission;
  }
  case Action::DEPOSIT: {
    auto* t = static_cast<const DepositTransaction*>(transaction);

    return t->amount;
  }
  case Action::WITHDRAW: {
    auto* t = static_cast<const WithdrawTransaction*>(transaction);
    return -t->amount;
  }
  case Action::DIVIDEND: {
    return static_cast<const DepositTransaction*>(transaction)->amount;
  }
  default:
    return 0;
  }
}

Security* security(const Transaction* transaction) {
  switch (transaction->action()) {
  case Action::BUY:
    return static_cast<const BuyTransaction*>(transaction)->security;
  case Action::SELL:
    return static_cast<const SellTransaction*>(transaction)->security;
  case Action::DEPOSIT:
    return static_cast<const DepositTransaction*>(transaction)->security;
  case Action::WITHDRAW:
    return static_cast<const WithdrawTransaction*>(transaction)->security;
  case Action::DIVIDEND:
    return static_cast<const DividendTransaction*>(transaction)->security;
  default:
    return nullptr;
  }
}

} // namespace algorithms

} // namespace pv
