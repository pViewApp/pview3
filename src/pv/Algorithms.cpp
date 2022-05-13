#include "Algorithms.h"
#include "pv/Action.h"
#include "pv/Actions.h"
#include "pv/Transaction.h"
#include <algorithm>
#include <execution>
#include <valarray>
#include <vector>

namespace pv {
namespace algorithms {

Decimal cashBalance(const Account& account, Date date) {
  std::vector<Transaction> transactions;
  std::copy_if(account.transactions().cbegin(), account.transactions().cend(), std::back_inserter(transactions),
               [&date](const Transaction& t) { return t.date() <= date; });

  if (transactions.size() == 0)
    return 0;

  std::valarray<Decimal> cashBalances(transactions.size());
  std::transform(transactions.cbegin(), transactions.cend(), std::begin(cashBalances),
                 [](const Transaction& transaction) { return transaction.action().cashBalance(transaction); });

  return cashBalances.sum();
}

Decimal sharesHeld(const Security& security, const Account& account, Date date) {
  std::vector<pv::Decimal> numberOfSharesPerTransaction;
  for (const auto& t : account.transactions()) {

    if (t.date() <= date && t.security() == security) {
      numberOfSharesPerTransaction.push_back(t.action().numberOfShares(t));
    }
  }

  return std::reduce(numberOfSharesPerTransaction.cbegin(), numberOfSharesPerTransaction.cend());
}

Decimal sharesHeld(const Security& security, Date date) {
  if (security.dataFile()->accounts().size() == 0) {
    return 0;
  }

  std::valarray<Decimal> perAccount(security.dataFile()->accounts().size());
  std::transform(security.dataFile()->accounts().cbegin(), security.dataFile()->accounts().cend(),
                 std::begin(perAccount),
                 [&security, &date](const Account& account) { return sharesHeld(security, account, date); });

  return perAccount.sum();
}

std::optional<Decimal> sharePrice(const Security& security, Date date) {
  auto iter = security.prices().lower_bound(date);
  if (iter == security.prices().cend()) {
    return std::nullopt;
  }

  if (iter->first == date) {
    return iter->second;
  }

  if (iter == security.prices().cbegin()) {
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
  if (security.dataFile()->accounts().size() == 0) {
    return 0;
  }

  std::valarray<Decimal> perAccount(security.dataFile()->accounts().size());
  std::transform(security.dataFile()->accounts().cbegin(), security.dataFile()->accounts().cend(),
                 std::begin(perAccount),
                 [&security, &date](const Account& account) { return cashSpent(security, account, date); });

  return perAccount.sum();
}

Decimal cashSpent(const Security& security, const Account& account, Date date) {
  std::vector<pv::Decimal> cashSpentPerTransaction;
  for (const auto& t : account.transactions()) {
    if (t.date() <= date && &t.action() == &pv::actions::BUY && t.security() == security) {
      cashSpentPerTransaction.push_back(-(t.action().cashBalance(t)));
    }
  }

  return std::reduce(cashSpentPerTransaction.cbegin(), cashSpentPerTransaction.cend());
}

Decimal cashEarned(const Security& security, const Account& account, Date date) {
  std::vector<pv::Decimal> cashEarnedPerTransaction;
  for (const auto& t : account.transactions()) {
    if (t.date() <= date && &t.action() == &pv::actions::SELL && t.security() == security) {
      cashEarnedPerTransaction.push_back(t.action().cashBalance(t));
    }
  }

  return std::reduce(cashEarnedPerTransaction.cbegin(), cashEarnedPerTransaction.cend());
}

Decimal cashEarned(const Security& security, Date date) {
  if (security.dataFile()->accounts().size() == 0) {
    return 0;
  }

  std::valarray<Decimal> perAccount(security.dataFile()->accounts().size());
  std::transform(security.dataFile()->accounts().cbegin(), security.dataFile()->accounts().cend(),
                 std::begin(perAccount),
                 [&security, &date](const Account& account) { return cashEarned(security, account, date); });

  return perAccount.sum();
}

Decimal cashGained(const Security& security, const Account& account, Date date) {
  return cashEarned(security, account, date) - cashSpent(security, account, date);
}

Decimal cashGained(const Security& security, Date date) {
  return cashEarned(security, date) - cashSpent(security, date);
}

std::optional<Decimal> averageBuyPrice(const Security& security, Date date) {
  std::vector<Decimal> buyPrices;
  for (const auto& account : security.dataFile()->accounts()) {
    for (const auto& transaction : account.transactions()) {
      if (&transaction.action() == &pv::actions::BUY && transaction.security() == security &&
          transaction.date() <= date) {
        buyPrices.push_back(transaction.sharePrice());
      }
    }
  }

  if (buyPrices.size() == 0) {
    return std::nullopt;
  }

  return std::reduce(buyPrices.cbegin(), buyPrices.cend()) / buyPrices.size();
}

std::optional<Decimal> averageSellPrice(const Security& security, Date date) {

  std::vector<Decimal> sellPrices;
  for (const auto& account : security.dataFile()->accounts()) {
    for (const auto& transaction : account.transactions()) {
      if (&transaction.action() == &pv::actions::SELL && transaction.security() == security &&
          transaction.date() <= date) {
        sellPrices.push_back(transaction.sharePrice());
      }
    }
  }

  if (sellPrices.size() == 0) {
    return std::nullopt;
  }

  return std::reduce(sellPrices.cbegin(), sellPrices.cend()) / sellPrices.size();
}

std::optional<Decimal> averageBuyPrice(const Security& security, const Account& account, Date date) {
  std::vector<Decimal> buyPrices;
  for (const auto& transaction : account.transactions()) {
    if (&transaction.action() == &pv::actions::BUY && transaction.security() == security &&
        transaction.date() <= date) {
      buyPrices.push_back(transaction.sharePrice());
    }
  }

  if (buyPrices.size() == 0) {
    return std::nullopt;
  }

  return std::reduce(buyPrices.cbegin(), buyPrices.cend()) / buyPrices.size();
}

std::optional<Decimal> averageSellPrice(const Security& security, const Account& account, Date date) {

  std::vector<Decimal> sellPrices;
  for (const auto& transaction : account.transactions()) {
    if (&transaction.action() == &pv::actions::SELL && transaction.security() == security &&
        transaction.date() <= date) {
      sellPrices.push_back(transaction.sharePrice());
    }
  }

  if (sellPrices.size() == 0) {
    return std::nullopt;
  }

  return std::reduce(sellPrices.cbegin(), sellPrices.cend()) / sellPrices.size();
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
    if (t.date() <= date && &t.action() == &pv::actions::DIVIDEND && t.security() == security) {
      dividendsPerTransaction.push_back(t.action().cashBalance(t));
    }
  }

  return std::reduce(dividendsPerTransaction.cbegin(), dividendsPerTransaction.cend());
}

Decimal dividendIncome(const Security& security, Date date) {
  const auto& accounts = security.dataFile()->accounts();
  if (accounts.size() == 0) {
    return 0;
  }

  std::valarray<pv::Decimal> dividendsPerAccount = {accounts.size()};
  std::transform(accounts.cbegin(), accounts.cend(), std::begin(dividendsPerAccount),
                 [&](const Account& account) { return dividendIncome(security, account, date); });

  return dividendsPerAccount.sum();
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

} // namespace algorithms
} // namespace pv
