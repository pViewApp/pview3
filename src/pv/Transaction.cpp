#include "Transaction.h"
#include "pv/DataFile.h"
#include "pv/Integer64.h"
#include <sqlite3.h>

namespace pv {
namespace transaction {

i64 date(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT Date FROM Transactions WHERE Id = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 account(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT AccountId FROM Transactions WHERE Id = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

pv::Action action(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT Action FROM Transactions WHERE Id = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return static_cast<pv::Action>(sqlite3_column_int(query, 0));
}

i64 buySecurity(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT SecurityId FROM BuyTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 buyNumberOfShares(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT NumberOfShares FROM BuyTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 buySharePrice(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT SharePrice FROM BuyTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 buyCommission(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT Commission FROM BuyTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 buyAmount(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT Amount FROM BuyTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 sellSecurity(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT SecurityId FROM SellTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 sellNumberOfShares(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT NumberOfShares FROM SellTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 sellSharePrice(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT SharePrice FROM SellTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 sellCommission(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT Commission FROM SellTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 sellAmount(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT Amount FROM SellTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

std::optional<i64> depositSecurity(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT SecurityId FROM DepositTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  if (sqlite3_column_type(query, 0) == SQLITE_NULL) {
    return std::nullopt;
  } else {
    return sqlite3_column_int64(query, 0);
  }
}

i64 depositAmount(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT Amount FROM DepositTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

std::optional<i64> withdrawSecurity(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT SecurityId FROM WithdrawTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  if (sqlite3_column_type(query, 0) == SQLITE_NULL) {
    return std::nullopt;
  } else {
    return sqlite3_column_int64(query, 0);
  }
}

i64 withdrawAmount(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT Amount FROM WithdrawTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 dividendSecurity(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT SecurityId FROM DividendTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 dividendAmount(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT Amount FROM DividendTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 interestSecurity(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT SecurityId FROM InterestTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}

i64 interestAmount(DataFile& dataFile, pv::i64 transaction) noexcept {
  auto* query = dataFile.cachedQuery("SELECT Amount FROM InterestTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(query);
  return sqlite3_column_int64(query, 0);
}
}
} // namespace pv
