#include "Transaction.h"
#include "pv/DataFile.h"
#include "pv/Integer64.h"
#include <sqlite3.h>

namespace pv {
namespace transaction {

i64 date(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT Date FROM Transactions WHERE Id = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

i64 account(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT AccountId FROM Transactions WHERE Id = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

pv::Action action(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT Action FROM Transactions WHERE Id = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return static_cast<pv::Action>(sqlite3_column_int(&*query, 0));
}

i64 buySecurity(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT SecurityId FROM BuyTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

i64 buyNumberOfShares(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT NumberOfShares FROM BuyTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

i64 buySharePrice(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT SharePrice FROM BuyTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

i64 buyCommission(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT Commission FROM BuyTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

i64 buyAmount(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT Amount FROM BuyTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

i64 sellSecurity(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT SecurityId FROM SellTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

i64 sellNumberOfShares(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT NumberOfShares FROM SellTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

i64 sellSharePrice(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT SharePrice FROM SellTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

i64 sellCommission(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT Commission FROM SellTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

i64 sellAmount(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT Amount FROM SellTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

std::optional<i64> depositSecurity(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT SecurityId FROM DepositTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  if (sqlite3_column_type(&*query, 0) == SQLITE_NULL) {
    return std::nullopt;
  } else {
    return sqlite3_column_int64(&*query, 0);
  }
}

i64 depositAmount(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT Amount FROM DepositTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

std::optional<i64> withdrawSecurity(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT SecurityId FROM WithdrawTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  if (sqlite3_column_type(&*query, 0) == SQLITE_NULL) {
    return std::nullopt;
  } else {
    return sqlite3_column_int64(&*query, 0);
  }
}

i64 withdrawAmount(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT Amount FROM WithdrawTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

i64 dividendSecurity(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT SecurityId FROM DividendTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

i64 dividendAmount(const DataFile& dataFile, pv::i64 transaction) noexcept {
  auto query = dataFile.query("SELECT Amount FROM DividendTransactions WHERE TransactionId = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(&*query);
  return sqlite3_column_int64(&*query, 0);
}

}
} // namespace pv
