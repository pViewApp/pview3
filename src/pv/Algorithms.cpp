#include "Algorithms.h"
#include <optional>
#include <sqlite3.h>
#include <cmath>

namespace {

const char* cashBalanceQuery = R"(
SELECT -COALESCE(SUM(BuyTransactions.Amount), 0) + COALESCE(SUM(SellTransactions.Amount), 0)
  + COALESCE(SUM(DepositTransactions.Amount), 0) - COALESCE(SUM(WithdrawTransactions.Amount), 0)
  + COALESCE(SUM(DividendTransactions.Amount), 0)
FROM Transactions
  LEFT JOIN BuyTransactions ON Transactions.Id = BuyTransactions.TransactionId
  LEFT JOIN SellTransactions ON Transactions.Id = SellTransactions.TransactionId
  LEFT JOIN DepositTransactions ON Transactions.Id = DepositTransactions.TransactionId
  LEFT JOIN WithdrawTransactions ON Transactions.Id = WithdrawTransactions.TransactionId
  LEFT JOIN DividendTransactions ON Transactions.Id = DividendTransactions.TransactionId
WHERE Transactions.AccountId = ? AND Transactions.Date <= ?
)";

const char* sharesHeldQuery = R"(
SELECT COALESCE(SUM(BuySell.NumberOfShares), 0) FROM Transactions
  INNER JOIN
    (SELECT TransactionId, SecurityId, NumberOfShares FROM BuyTransactions UNION ALL SELECT TransactionId, SecurityId, -NumberOfShares FROM SellTransactions) BuySell
    ON Transactions.Id = BuySell.TransactionId  AND BuySell.SecurityId = :SecurityId
WHERE Transactions.Date <= :Date
)";

const char* sharesHeldByAccountQuery = R"(
SELECT COALESCE(SUM(BuySell.NumberOfShares), 0) FROM Transactions
  INNER JOIN
    (SELECT TransactionId, SecurityId, NumberOfShares FROM BuyTransactions UNION ALL SELECT TransactionId, SecurityId, -NumberOfShares FROM SellTransactions) BuySell
    ON Transactions.Id = BuySell.TransactionId  AND BuySell.SecurityId = :SecurityId
WHERE Transactions.Date <= :Date AND Transaction.AccountId = :AccountId
)";

const char* sharesSoldQuery = R"(
SELECT COALESCE(SUM(SellTransactions.NumberOfShares), 0) FROM Transactions
  INNER JOIN SellTransactions
    ON Transactions.Id = SellTransactions.TransactionId  AND SellTransactions.SecurityId = :SecurityId
WHERE Transactions.Date <= :Date
)";

const char* sharesSoldByAccountQuery = R"(
SELECT COALESCE(SUM(SellTransactions.NumberOfShares), 0) FROM Transactions
  INNER JOIN SellTransactions
    ON Transactions.Id = SellTransactions.TransactionId  AND SellTransactions.SecurityId = :SecurityId
WHERE Transactions.Date <= :Date AND Transaction.AccountId = :AccountId
)";

const char* dividendIncomeQuery = R"(
SELECT COALESCE(SUM(DividendTransactions.Amount), 0) FROM Transactions
  INNER JOIN DividendTransactions ON Transactions.Id = DividendTransactions.TransactionId
    AND DividendTransactions.SecurityId = :SecurityId
WHERE Transactions.Date <= :Date
)";

const char* dividendIncomeByAccountQuery = R"(
SELECT COALESCE(SUM(DividendTransactions.Amount), 0) FROM Transactions
  INNER JOIN DividendTransactions ON Transactions.Id = DividendTransactions.TransactionId
    AND DividendTransactions.SecurityId = :SecurityId
WHERE Transactions.Date <= :Date AND Transactions.AccountId = :AccountId
)";

const char* interestIncomeQuery = R"(
SELECT COALESCE(SUM(InterestTransactions.Amount), 0) FROM Transactions
  INNER JOIN InterestTransactions ON Transactions.Id = InterestTransactions.TransactionId
    AND InterestTransactions.SecurityId = :SecurityId
WHERE Transactions.Date <= :Date
)";

const char* interestIncomeByAccountQuery = R"(
SELECT COALESCE(SUM(InterestTransactions.Amount), 0) FROM Transactions
  INNER JOIN InterestTransactions ON Transactions.Id = InterestTransactions.TransactionId
    AND InterestTransactions.SecurityId = :SecurityId
WHERE Transactions.Date <= :Date AND Transactions.AccountId = :AccountId
)";

const char* averageBuyPriceQuery = R"(
SELECT SUM(BuyTransactions.SharePrice * BuyTransactions.NumberOfShares) / SUM(BuyTransactions.NumberOfShares) FROM Transactions
  INNER JOIN BuyTransactions ON Transactions.Id = BuyTransactions.TransactionId
    AND BuyTransactions.SecurityId = :SecurityId
WHERE Transactions.Date <= :Date
)";

const char* averageBuyPriceByAccountQuery = R"(
SELECT SUM(BuyTransactions.SharePrice * BuyTransactions.NumberOfShares) / SUM(BuyTransactions.NumberOfShares) FROM Transactions
  INNER JOIN BuyTransactions ON Transactions.Id = BuyTransactions.TransactionId
    AND BuyTransactions.SecurityId = :SecurityId
WHERE Transactions.Date <= :Date AND Transactions.AccountId = :AccountId
)";

const char* averageSellPriceQuery = R"(
SELECT SUM(SellTransactions.SharePrice * SellTransactions.NumberOfShares) / SUM(SellTransactions.NumberOfShares) FROM Transactions
  INNER JOIN SellTransactions ON Transactions.Id = SellTransactions.TransactionId
    AND SellTransactions.SecurityId = :SecurityId
WHERE Transactions.Date <= :Date
)";

const char* averageSellPriceByAccountQuery = R"(
SELECT SUM(SellTransactions.SharePrice * SellTransactions.NumberOfShares) / SUM(SellTransactions.NumberOfShares) FROM Transactions
  INNER JOIN SellTransactions ON Transactions.Id = SellTransactions.TransactionId
    AND SellTransactions.SecurityId = :SecurityId
WHERE Transactions.Date <= :Date AND Transactions.AccountId = :AccountId
)";

const char* sharePriceQuery =
  "SELECT Price FROM SecurityPrices WHERE SecurityId = ? AND Date <= ? ORDER BY Date DESC LIMIT 1";

} // namespace

namespace pv {
namespace algorithms {

i64 cashBalance(DataFile& dataFile, i64 account, i64 date) {
  auto* stmt = dataFile.cachedQuery(cashBalanceQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(account));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_step(stmt);
  auto result = sqlite3_column_int64(stmt, 0);
  return result;
}

i64 sharesHeld(DataFile& dataFile, i64 security, i64 date) {
  auto* stmt = dataFile.cachedQuery(sharesHeldQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_step(stmt);
  auto result = sqlite3_column_int64(stmt, 0);
  return result;
}

i64 sharesHeld(DataFile& dataFile, i64 security, i64 account, i64 date) {
  auto* stmt = dataFile.cachedQuery(sharesHeldByAccountQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(account));
  sqlite3_step(stmt);
  auto result = sqlite3_column_int64(stmt, 0);
  return result;
}

i64 sharesSold(DataFile& dataFile, i64 security, i64 date) {
  auto* stmt = dataFile.cachedQuery(sharesSoldQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_step(stmt);
  auto result = sqlite3_column_int64(stmt, 0);
  return result;
}

i64 sharesSold(DataFile& dataFile, i64 security, i64 account, i64 date) {
  auto* stmt = dataFile.cachedQuery(sharesSoldByAccountQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(account));
  sqlite3_step(stmt);
  auto result = sqlite3_column_int64(stmt, 0);
  return result;
}

i64 cashGained(DataFile& dataFile, i64 security, i64 date) {
  return sharesSold(dataFile, security, date) * (averageSellPrice(dataFile, security, date).value_or(0) - averageBuyPrice(dataFile, security, date).value_or(0));
} 

i64 cashGained(DataFile& dataFile, i64 security, i64 account, i64 date) {
  return sharesSold(dataFile, security, date) * (averageSellPrice(dataFile, security, account, date).value_or(0) - averageBuyPrice(dataFile, security, account, date).value_or(0));
}

i64 dividendIncome(DataFile& dataFile, i64 security, i64 date) {
  auto* stmt = dataFile.cachedQuery(dividendIncomeQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_step(stmt);
  auto result = sqlite3_column_int64(stmt, 0);
  return result;
}

i64 dividendIncome(DataFile& dataFile, i64 security, i64 account, i64 date) {
  auto* stmt = dataFile.cachedQuery(dividendIncomeByAccountQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(account));
  sqlite3_step(stmt);
  auto result = sqlite3_column_int64(stmt, 0);
  return result;
}

i64 interestIncome(DataFile& dataFile, i64 security, i64 date) {
  auto* stmt = dataFile.cachedQuery(interestIncomeQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_step(stmt);
  auto result = sqlite3_column_int64(stmt, 0);
  return result;
}

i64 interestIncome(DataFile& dataFile, i64 security, i64 account, i64 date) {
  auto* stmt = dataFile.cachedQuery(interestIncomeByAccountQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(account));
  sqlite3_step(stmt);
  auto result = sqlite3_column_int64(stmt, 0);
  return result;
}

i64 costBasis(DataFile& dataFile, i64 security, i64 date) {
  return sharesHeld(dataFile, security, date) * averageBuyPrice(dataFile, security, date).value_or(0);
}

i64 costBasis(DataFile& dataFile, i64 security, i64 account, i64 date) {
  return sharesHeld(dataFile, security, account, date) * averageBuyPrice(dataFile, security, account, date).value_or(0);
}

i64 totalIncome(DataFile& dataFile, i64 security, i64 date) {
  return unrealizedCashGained(dataFile, security, date).value_or(0) + cashGained(dataFile, security, date) + dividendIncome(dataFile, security, date) + interestIncome(dataFile, security, date);
}

i64 totalIncome(DataFile& dataFile, i64 security, i64 account, i64 date) {
  return unrealizedCashGained(dataFile, security, account, date).value_or(0) + cashGained(dataFile, security, account, date) + dividendIncome(dataFile, security, account, date) + interestIncome(dataFile, security, date);
}

std::optional<i64> sharePrice(DataFile& dataFile, i64 security, i64 date) {
  auto* stmt = dataFile.cachedQuery(sharePriceQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_step(stmt);
  std::optional<i64> result = std::nullopt;
  
  if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
    result = sqlite3_column_int64(stmt, 0);
  }

  return result;
}


std::optional<i64> unrealizedCashGained(DataFile& dataFile, i64 security, i64 date) {
  auto sharePrice_ = sharePrice(dataFile, security, date);
  auto averageBuyPrice_ = averageBuyPrice(dataFile, security, date);
  if (!sharePrice_.has_value() || !averageBuyPrice_.has_value()) {
    return std::nullopt;
  }
  return sharesHeld(dataFile, security, date) * (sharePrice_.value() - averageBuyPrice_.value());
}

std::optional<i64> unrealizedCashGained(DataFile& dataFile, i64 security, i64 account, i64 date) {
  auto sharePrice_ = sharePrice(dataFile, security, date);
  auto averageBuyPrice_ = averageBuyPrice(dataFile, security, account, date);
  if (!sharePrice_.has_value() || !averageBuyPrice_.has_value()) {
    return std::nullopt;
  }
  return sharesHeld(dataFile, security, account, date) * (sharePrice_.value() - averageBuyPrice_.value());
}

std::optional<i64> averageBuyPrice(DataFile& dataFile, i64 security, i64 date) {
  auto* stmt = dataFile.cachedQuery(averageBuyPriceQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_step(stmt);
  std::optional<i64> result = std::nullopt;
  
  if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
    result = std::llround(sqlite3_column_double(stmt, 0));
  }

  return result;
}

std::optional<i64> averageBuyPrice(DataFile& dataFile, i64 security, i64 account, i64 date) {
  auto* stmt = dataFile.cachedQuery(averageBuyPriceByAccountQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(account));
  sqlite3_step(stmt);
  std::optional<i64> result = std::nullopt;
  
  if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
    result = std::llround(sqlite3_column_double(stmt, 0));
  }

  return result;
}


std::optional<i64> averageSellPrice(DataFile& dataFile, i64 security, i64 date) {
  auto* stmt = dataFile.cachedQuery(averageSellPriceQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_step(stmt);
  std::optional<i64> result = std::nullopt;
  
  if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
    result = std::llround(sqlite3_column_double(stmt, 0));
  }

  return result;
}

std::optional<i64> averageSellPrice(DataFile& dataFile, i64 security, i64 account, i64 date) {
  auto* stmt = dataFile.cachedQuery(averageSellPriceByAccountQuery);
  if (!stmt) {
    return 0;
  }
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(account));
  sqlite3_step(stmt);
  std::optional<i64> result = std::nullopt;
  
  if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
    result = std::llround(sqlite3_column_double(stmt, 0));
  }

  return result;
}

std::optional<i64> marketValue(DataFile& dataFile, i64 security, i64 date) {
  auto sharePrice_ = sharePrice(dataFile, security, date);
  if (!sharePrice_.has_value()) {
    return std::nullopt;
  }
  return (*sharePrice_) * sharesHeld(dataFile, security, date);
}

std::optional<i64> marketValue(DataFile& dataFile, i64 security, i64 account, i64 date) {
  auto sharePrice_ = sharePrice(dataFile, security, date);
  if (!sharePrice_.has_value()) {
    return std::nullopt;
  }
  return (*sharePrice_) * sharesHeld(dataFile, security, account, date);
}

} // namespace algorithms

} // namespace pv
