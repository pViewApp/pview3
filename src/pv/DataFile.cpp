#include "DataFile.h"
#include <algorithm>
#include <optional>
#include <sqlite3.h>
#include <cassert>
#include <stdexcept>
#include <string>

namespace pv {

namespace {

StatementPointer createStatementPointer(sqlite3_stmt* stmt) { return StatementPointer(stmt, &sqlite3_finalize); }

constexpr char initializationSQL[] = R"(
PRAGMA foreign_keys = ON;
PRAGMA application_id = 1347831366;
PRAGMA user_version = 3;

CREATE TABLE IF NOT EXISTS Accounts(
  Id INTEGER NOT NULL PRIMARY KEY,
  Name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS Securities(
  Id INTEGER NOT NULL PRIMARY KEY,
  Symbol TEXT NOT NULL,
  Name TEXT NOT NULL,
  AssetClass TEXT NOT NULL,
  Sector TEXT NOT NULL,
  
  UNIQUE(Symbol)
);

CREATE TABLE IF NOT EXISTS SecurityPrices(
  SecurityId INTEGER NOT NULL,
  Date INTEGER NOT NULL,
  Price INTEGER NOT NULL,

  FOREIGN KEY(SecurityId) REFERENCES Securities(Id) ON DELETE CASCADE,
  PRIMARY KEY(SecurityId, Date)
) WITHOUT ROWID;

CREATE TABLE IF NOT EXISTS Transactions(
  AccountId INTEGER NOT NULL,
  Id INTEGER NOT NULL PRIMARY KEY,
  Date INTEGER NOT NULL,
  Action INTEGER NOT NULL,

  FOREIGN KEY(AccountId) REFERENCES Accounts(Id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS BuyTransactions(
  TransactionId INTEGER NOT NULL PRIMARY KEY,
  SecurityId INTEGER NOT NULL,
  NumberOfShares INTEGER NOT NULL,
  SharePrice INTEGER NOT NULL,
  Commission INTEGER NOT NULL,
  
  Amount INTEGER GENERATED ALWAYS AS ((NumberOfShares * SharePrice) + Commission) STORED,

  CHECK(NumberOfShares >= 0),
  CHECK(SharePrice >= 0),
  CHECK(Commission >= 0),
  FOREIGN KEY(TransactionId) REFERENCES Transactions(Id) ON DELETE CASCADE,
  FOREIGN KEY(SecurityId) REFERENCES Securities(Id)
);

CREATE TABLE IF NOT EXISTS SellTransactions (
  TransactionId INTEGER NOT NULL PRIMARY KEY,
  SecurityId INTEGER NOT NULL,
  NumberOfShares INTEGER NOT NULL,
  SharePrice INTEGER NOT NULL,
  Commission INTEGER NOT NULL,
  
  Amount INTEGER GENERATED ALWAYS AS ((NumberOfShares * SharePrice) - Commission) STORED,

  CHECK(NumberOfShares >= 0),
  CHECK(SharePrice >= 0),
  CHECK(Commission >= 0),
  FOREIGN KEY(TransactionId) REFERENCES Transactions(Id) ON DELETE CASCADE,
  FOREIGN KEY(SecurityId) REFERENCES Securities(Id)
);

CREATE TABLE IF NOT EXISTS DepositTransactions(
  TransactionId INTEGER NOT NULL PRIMARY KEY,
  SecurityId INTEGER,
  Amount INTEGER NOT NULL,

  CHECK(Amount >= 0),
  FOREIGN KEY(TransactionId) REFERENCES Transactions(Id) ON DELETE CASCADE,
  FOREIGN KEY(SecurityId) REFERENCES Securities(Id)
);

CREATE TABLE IF NOT EXISTS WithdrawTransactions(
  TransactionId INTEGER NOT NULL PRIMARY KEY,
  SecurityId INTEGER,
  Amount INTEGER NOT NULL,

  CHECK(Amount >= 0),
  FOREIGN KEY(TransactionId) REFERENCES Transactions(Id) ON DELETE CASCADE,
  FOREIGN KEY(SecurityId) REFERENCES Securities(Id)
);

CREATE TABLE IF NOT EXISTS DividendTransactions(
  TransactionId INTEGER NOT NULL PRIMARY KEY,
  SecurityId INTEGER NOT NULL,
  Amount INTEGER NOT NULL,
  
  CHECK(Amount >= 0),
  FOREIGN KEY(TransactionId) REFERENCES Transactions(Id) ON DELETE CASCADE,
  FOREIGN KEY(SecurityId) REFERENCES Securities(Id)
);

CREATE INDEX IF NOT EXISTS TransactionsIndex ON Transactions(Date, AccountId);
CREATE INDEX IF NOT EXISTS BuyTransactionsSecurityIndex ON BuyTransactions(SecurityId);
CREATE INDEX IF NOT EXISTS SellTransactionsSecurityIndex ON SellTransactions(SecurityId);
CREATE INDEX IF NOT EXISTS DepositTransactionsSecurityIndex ON DepositTransactions(SecurityId);
CREATE INDEX IF NOT EXISTS WithdrawTransactionsSecurityIndex ON WithdrawTransactions(SecurityId);
CREATE INDEX IF NOT EXISTS DividendTransactionsSecurityIndex ON DividendTransactions(SecurityId);
)";

/// \internal Maps and SQLite result code to it's pv::ResultCode equivalant.
ResultCode mapSQLiteCodes(int code) {
  switch (code) {
  case SQLITE_OK:
    return ResultCode::OK;
  case SQLITE_ROW:
    return ResultCode::OK;
  case SQLITE_DONE:
    return ResultCode::OK;
  case SQLITE_NOMEM:
    return ResultCode::SQL_NOMEM;
  case SQLITE_CORRUPT:
    return ResultCode::SQL_CORRUPT;
  case SQLITE_ERROR:
    return ResultCode::SQL_ERROR;
  case SQLITE_CONSTRAINT:
    return ResultCode::SQL_CONSTRAINT;
  default:
    return ResultCode::SQL_ERROR;
  };
}

} // namespace

DataFile::DataFile(std::string location, int flags) {
  if (flags == -1) {
    flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
  }

  auto result = sqlite3_open_v2(location.c_str(), &db, flags, nullptr);
  if (result != SQLITE_OK) {
    throw std::runtime_error(std::string("Failed to open DataFile: SQLite Error Code ") +
                             std::to_string((static_cast<int>(result))));
  }

  result = sqlite3_exec(db, initializationSQL, nullptr, nullptr, nullptr);

  if (result != SQLITE_OK) {
    throw std::runtime_error(std::string("Failed to initialize DataFile: SQLite Error Code ") +
                             std::to_string((static_cast<int>(result))));
  }

  //// Accounts & Securities

  stmt_addAccount = prepare("INSERT INTO Accounts(Name) VALUES(?)", SQLITE_PREPARE_PERSISTENT);
  stmt_addSecurity = prepare("INSERT INTO Securities(Symbol, Name, AssetClass, Sector) VALUES (?, ?, ?, ?)",
                             SQLITE_PREPARE_PERSISTENT);

  stmt_removeAccount = prepare("DELETE FROM Accounts WHERE Id = ?", SQLITE_PREPARE_PERSISTENT);
  stmt_removeSecurity = prepare("DELETE FROM Securities WHERE Id = ?", SQLITE_PREPARE_PERSISTENT);

  stmt_setAccountName = prepare("UPDATE Accounts SET Name = ? WHERE Id = ?", SQLITE_PREPARE_PERSISTENT);

  stmt_setSecurityName = prepare("UPDATE Securities SET Name = ? WHERE Id = ?", SQLITE_PREPARE_PERSISTENT);
  stmt_setSecurityAssetClass = prepare("UPDATE Securities SET AssetClass = ? WHERE Id = ?", SQLITE_PREPARE_PERSISTENT);
  stmt_setSecuritySector = prepare("UPDATE Securities SET Sector = ? WHERE Id = ?", SQLITE_PREPARE_PERSISTENT);

  //// Inserting Transactions

  stmt_addTransaction =
      prepare("INSERT INTO Transactions(AccountId, Date, Action) VALUES (?, ?, ?)", SQLITE_PREPARE_PERSISTENT);

  stmt_addBuyTransaction = prepare("INSERT INTO BuyTransactions(TransactionId, SecurityId, NumberOfShares, SharePrice, "
                                   "Commission) VALUES (?, ?, ?, ?, ?)",
                                   SQLITE_PREPARE_PERSISTENT);
  stmt_addSellTransaction = prepare("INSERT INTO SellTransactions(TransactionId, SecurityId, NumberOfShares, "
                                    "SharePrice, Commission) VALUES (?, ?, ?, ?, ?)",
                                    SQLITE_PREPARE_PERSISTENT);
  stmt_addDepositTransaction = prepare(
      "INSERT INTO DepositTransactions(TransactionId, SecurityId, Amount) VALUES (?, ?, ?)", SQLITE_PREPARE_PERSISTENT);
  stmt_addWithdrawTransaction =
      prepare("INSERT INTO WithdrawTransactions(TransactionId, SecurityId, Amount) VALUES (?, ?, ?)",
              SQLITE_PREPARE_PERSISTENT);
  stmt_addDividendTransaction =
      prepare("INSERT INTO DividendTransactions(TransactionId, SecurityId, Amount) VALUES (?, ?, ?)",
              SQLITE_PREPARE_PERSISTENT);

  //// Transaction Updates

  stmt_setBuyNumberOfShares =
      prepare("UPDATE BuyTransactions SET NumberOfShares = ? WHERE TransactionId = ?", SQLITE_PREPARE_PERSISTENT);
  stmt_setBuySharePrice =
      prepare("UPDATE BuyTransactions SET SharePrice = ? WHERE TransactionId = ?", SQLITE_PREPARE_PERSISTENT);
  stmt_setBuyCommission =
      prepare("UPDATE BuyTransactions SET Commission = ? WHERE TransactionId = ?", SQLITE_PREPARE_PERSISTENT);

  stmt_setSellNumberOfShares =
      prepare("UPDATE SellTransactions SET NumberOfShares = ? WHERE TransactionId = ?", SQLITE_PREPARE_PERSISTENT);
  stmt_setSellSharePrice =
      prepare("UPDATE SellTransactions SET SharePrice = ? WHERE TransactionId = ?", SQLITE_PREPARE_PERSISTENT);
  stmt_setSellCommission =
      prepare("UPDATE SellTransactions SET Commission = ? WHERE TransactionId = ?", SQLITE_PREPARE_PERSISTENT);

  stmt_setDepositAmount =
      prepare("UPDATE DepositTransactions SET Amount = ? WHERE TransactionId = ?", SQLITE_PREPARE_PERSISTENT);
  stmt_setWithdrawAmount =
      prepare("UPDATE WithdrawTransactions SET Amount = ? WHERE TransactionId = ?", SQLITE_PREPARE_PERSISTENT);
  stmt_setDividendAmount =
      prepare("UPDATE DividendTransactions SET Amount = ? WHERE TransactionId = ?", SQLITE_PREPARE_PERSISTENT);

  //// Removing Transactions

  stmt_removeTransaction = prepare("DELETE FROM Transactions WHERE Id = ?", SQLITE_PREPARE_PERSISTENT);

  //// Security Prices

  stmt_setSecurityPrice = prepare("INSERT INTO SecurityPrices(SecurityId, Date, Price) VALUES (?, ?, ?) ON CONFLICT DO "
                                  "UPDATE SET Price = excluded.Price",
                                  SQLITE_PREPARE_PERSISTENT);
  stmt_removeSecurityPrice =
      prepare("DELETE FROM SecurityPrices WHERE SecurityId = ? AND Date = ?", SQLITE_PREPARE_PERSISTENT);

  //// SQL Transactions and Savepoints

  stmt_beginTransaction = prepare("BEGIN TRANSACTION", SQLITE_PREPARE_PERSISTENT);
  stmt_rollbackTransaction = prepare("ROLLBACK TRANSACTION", SQLITE_PREPARE_PERSISTENT);
  stmt_commitTransaction = prepare("COMMIT TRANSACTION", SQLITE_PREPARE_PERSISTENT);

  stmt_vacuumInto = prepare("VACUUM INTO ?", SQLITE_PREPARE_PERSISTENT);

  stmt_beginSavepoint = prepare("BEGIN SAVEPOINT PVSavepoint", SQLITE_PREPARE_PERSISTENT);
  stmt_rollbackSavepoint = prepare("ROLLBACK TO PVSavepoint", SQLITE_PREPARE_PERSISTENT);
  stmt_releaseSavepoint = prepare("RELEASE PVSavepoint", SQLITE_PREPARE_PERSISTENT);

  updateSQLiteHooks();
}

DataFile::DataFile(DataFile&& other) noexcept { swap(*this, other); }

DataFile& DataFile::operator=(DataFile&& other) noexcept {
  DataFile temp(std::move(other));
  swap(*this, temp);
  return *this;
}

DataFile::~DataFile() noexcept {
  if (db == nullptr) return;

  // Close/finalize everything

  sqlite3_finalize(stmt_addAccount);
  sqlite3_finalize(stmt_addSecurity);
  sqlite3_finalize(stmt_removeAccount);
  sqlite3_finalize(stmt_removeSecurity);
  sqlite3_finalize(stmt_setAccountName);
  sqlite3_finalize(stmt_setSecurityName);
  sqlite3_finalize(stmt_setSecurityAssetClass);
  sqlite3_finalize(stmt_setSecuritySector);
  sqlite3_finalize(stmt_addTransaction);
  sqlite3_finalize(stmt_addBuyTransaction);
  sqlite3_finalize(stmt_addSellTransaction);
  sqlite3_finalize(stmt_addDepositTransaction);
  sqlite3_finalize(stmt_addWithdrawTransaction);
  sqlite3_finalize(stmt_addDividendTransaction);
  sqlite3_finalize(stmt_setBuyNumberOfShares);
  sqlite3_finalize(stmt_setBuySharePrice);
  sqlite3_finalize(stmt_setBuyCommission);
  sqlite3_finalize(stmt_setSellNumberOfShares);
  sqlite3_finalize(stmt_setSellSharePrice);
  sqlite3_finalize(stmt_setSellCommission);
  sqlite3_finalize(stmt_setDepositAmount);
  sqlite3_finalize(stmt_setWithdrawAmount);
  sqlite3_finalize(stmt_setDividendAmount);
  sqlite3_finalize(stmt_removeTransaction);
  sqlite3_finalize(stmt_setSecurityPrice);
  sqlite3_finalize(stmt_removeSecurityPrice);
  sqlite3_finalize(stmt_beginTransaction);
  sqlite3_finalize(stmt_rollbackTransaction);
  sqlite3_finalize(stmt_commitTransaction);
  sqlite3_finalize(stmt_vacuumInto);
  sqlite3_finalize(stmt_beginSavepoint);
  sqlite3_finalize(stmt_rollbackSavepoint);
  sqlite3_finalize(stmt_releaseSavepoint);

  sqlite3_close_v2(db);
}

void swap(DataFile& lhs, DataFile& rhs) noexcept {
  using std::swap;

  swap(lhs.changedSignal, rhs.changedSignal);
  swap(lhs.accountAddedSignal, rhs.accountAddedSignal);
  swap(lhs.accountUpdatedSignal, rhs.accountUpdatedSignal);
  swap(lhs.accountRemovedSignal, rhs.accountRemovedSignal);
  swap(lhs.transactionAddedSignal, rhs.transactionAddedSignal);
  swap(lhs.transactionUpdatedSignal, rhs.transactionUpdatedSignal);
  swap(lhs.transactionRemovedSignal, rhs.transactionRemovedSignal);
  swap(lhs.securityAddedSignal, rhs.securityAddedSignal);
  swap(lhs.securityUpdatedSignal, rhs.securityUpdatedSignal);
  swap(lhs.securityRemovedSignal, rhs.securityRemovedSignal);
  swap(lhs.securityPriceUpdatedSignal, rhs.securityPriceUpdatedSignal);
  swap(lhs.securityPriceRemovedSignal, rhs.securityPriceRemovedSignal);
  swap(lhs.rollbackSignal, rhs.rollbackSignal);
  swap(lhs.suppressRollbackSignal, rhs.suppressRollbackSignal);

  swap(lhs.db, rhs.db);
  swap(lhs.stmt_addAccount, rhs.stmt_addAccount);
  swap(lhs.stmt_addSecurity, rhs.stmt_addSecurity);
  swap(lhs.stmt_removeAccount, rhs.stmt_removeAccount);
  swap(lhs.stmt_removeSecurity, rhs.stmt_removeSecurity);
  swap(lhs.stmt_setAccountName, rhs.stmt_setAccountName);
  swap(lhs.stmt_setSecurityName, rhs.stmt_setSecurityName);
  swap(lhs.stmt_setSecurityAssetClass, rhs.stmt_setSecurityAssetClass);
  swap(lhs.stmt_setSecuritySector, rhs.stmt_setSecuritySector);
  swap(lhs.stmt_addTransaction, rhs.stmt_addTransaction);
  swap(lhs.stmt_addBuyTransaction, rhs.stmt_addBuyTransaction);
  swap(lhs.stmt_addSellTransaction, rhs.stmt_addSellTransaction);
  swap(lhs.stmt_addDepositTransaction, rhs.stmt_addDepositTransaction);
  swap(lhs.stmt_addWithdrawTransaction, rhs.stmt_addWithdrawTransaction);
  swap(lhs.stmt_addDividendTransaction, rhs.stmt_addDividendTransaction);
  swap(lhs.stmt_setBuyNumberOfShares, rhs.stmt_setBuyNumberOfShares);
  swap(lhs.stmt_setBuySharePrice, rhs.stmt_setBuySharePrice);
  swap(lhs.stmt_setBuyCommission, rhs.stmt_setBuyCommission);
  swap(lhs.stmt_setSellNumberOfShares, rhs.stmt_setSellNumberOfShares);
  swap(lhs.stmt_setSellSharePrice, rhs.stmt_setSellSharePrice);
  swap(lhs.stmt_setSellCommission, rhs.stmt_setSellCommission);
  swap(lhs.stmt_setDepositAmount, rhs.stmt_setDepositAmount);
  swap(lhs.stmt_setWithdrawAmount, rhs.stmt_setWithdrawAmount);
  swap(lhs.stmt_setDividendAmount, rhs.stmt_setDividendAmount);
  swap(lhs.stmt_removeTransaction, rhs.stmt_removeTransaction);
  swap(lhs.stmt_setSecurityPrice, rhs.stmt_setSecurityPrice);
  swap(lhs.stmt_removeSecurityPrice, rhs.stmt_removeSecurityPrice);
  swap(lhs.stmt_beginTransaction, rhs.stmt_beginTransaction);
  swap(lhs.stmt_rollbackTransaction, rhs.stmt_rollbackTransaction);
  swap(lhs.stmt_commitTransaction, rhs.stmt_commitTransaction);
  swap(lhs.stmt_vacuumInto, rhs.stmt_vacuumInto);
  swap(lhs.stmt_beginSavepoint, rhs.stmt_beginSavepoint);
  swap(lhs.stmt_rollbackSavepoint, rhs.stmt_rollbackSavepoint);
  swap(lhs.stmt_releaseSavepoint, rhs.stmt_releaseSavepoint);

  lhs.updateSQLiteHooks();
  rhs.updateSQLiteHooks();
}

void DataFile::updateSQLiteHooks()
{
  if (db == nullptr) {
    return;
  }
  sqlite3_rollback_hook(
      db,
      [](void* dataFilePtr) {
        auto* dataFile = static_cast<DataFile*>(dataFilePtr);
        dataFile->changedSignal();
        if (!dataFile->suppressRollbackSignal) {
          dataFile->rollbackSignal();
        }
      },
      this);

  sqlite3_update_hook(
      db,
      [](void* dataFilePtr, int, const char*, const char*, sqlite3_int64) {
        auto* dataFile = static_cast<DataFile*>(dataFilePtr);
        dataFile->changedSignal();
      },
      this
    );
}
sqlite3_stmt* DataFile::prepare(std::string sql, int flags, ResultCode* outResult) noexcept {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_stmt* stmt;
  auto result = sqlite3_prepare_v3(db, sql.c_str(), static_cast<int>(sql.length()), flags, &stmt, nullptr);
  if (outResult != nullptr) {
    (*outResult) = mapSQLiteCodes(result);
  }

  return stmt;
}

ResultCode DataFile::finishTransactionUpdate(ResultCode code, pv::i64 transaction) noexcept {
  if (code != ResultCode::OK) {
    return code;
  } else {
    auto changes = sqlite3_changes(db);
    assert(changes <= 1 && "Multiple rows changed when modifying transaction, how??");

    if (changes == 0) {
      return ResultCode::RECORD_NOT_FOUND;
    } else {
      transactionUpdatedSignal(transaction);
      return ResultCode::OK;
    }
  }
}

ResultCode DataFile::beginSavepoint() {
  sqlite3_step(stmt_beginSavepoint);
  return mapSQLiteCodes(sqlite3_reset(stmt_beginSavepoint));
}

ResultCode DataFile::rollbackSavepoint() {
  suppressRollbackSignal = true;
  sqlite3_step(stmt_rollbackSavepoint);
  suppressRollbackSignal = false;
  sqlite3_reset(stmt_rollbackSavepoint);

  auto result = releaseSavepoint();
  return result;
}

ResultCode DataFile::releaseSavepoint() {
  sqlite3_step(stmt_releaseSavepoint);
  return mapSQLiteCodes(sqlite3_reset(stmt_releaseSavepoint));
}

StatementPointer DataFile::query(std::string sql, int flags, ResultCode* outResult) const noexcept {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_stmt* stmt;
  auto result = sqlite3_prepare_v3(db, sql.c_str(), static_cast<int>(sql.size()), flags, &stmt, nullptr);

  if (outResult != nullptr) {
    (*outResult) = mapSQLiteCodes(result);
  }

  if (stmt != nullptr && !sqlite3_stmt_readonly(stmt)) {
    // If not readonly, give error
    if (outResult != nullptr) {
      (*outResult) = ResultCode::MODIFICATION_PROHIBITED;
    }
    sqlite3_finalize(stmt);
    return createStatementPointer(nullptr);
  }

  return createStatementPointer(stmt);
}

ResultCode DataFile::addAccount(std::string name) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_text(stmt_addAccount, 1, name.c_str(), static_cast<int>(name.length()), SQLITE_STATIC);
  auto result = mapSQLiteCodes(sqlite3_step(stmt_addAccount));
  sqlite3_reset(stmt_addAccount);
  sqlite3_clear_bindings(stmt_addAccount);

  if (result == ResultCode::OK) {
    accountAddedSignal(lastInsertedId());
  }

  return result;
}

ResultCode DataFile::addSecurity(std::string symbol, std::string name, std::string assetClass, std::string sector) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_text(stmt_addSecurity, 1, symbol.c_str(), static_cast<int>(symbol.length()), SQLITE_STATIC);
  sqlite3_bind_text(stmt_addSecurity, 2, name.c_str(), static_cast<int>(name.length()), SQLITE_STATIC);
  sqlite3_bind_text(stmt_addSecurity, 3, assetClass.c_str(), static_cast<int>(assetClass.length()), SQLITE_STATIC);
  sqlite3_bind_text(stmt_addSecurity, 4, sector.c_str(), static_cast<int>(sector.length()), SQLITE_STATIC);

  auto result = mapSQLiteCodes(sqlite3_step(stmt_addSecurity));

  sqlite3_reset(stmt_addSecurity);
  sqlite3_clear_bindings(stmt_addSecurity);

  if (result == ResultCode::OK) {
    securityAddedSignal(lastInsertedId());
  }

  return result;
}

ResultCode DataFile::removeAccount(i64 id) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_removeAccount, 1, static_cast<sqlite3_int64>(id));
  sqlite3_step(stmt_removeAccount);
  auto result = mapSQLiteCodes(sqlite3_reset(stmt_removeAccount));

  if (result == ResultCode::OK) {
    accountRemovedSignal(id);
  }

  return result;
}

ResultCode DataFile::removeSecurity(i64 id) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_removeSecurity, 1, static_cast<sqlite3_int64>(id));
  sqlite3_step(stmt_removeSecurity);
  auto result = mapSQLiteCodes(sqlite3_reset(stmt_removeSecurity));

  if (result == ResultCode::OK) {
    securityRemovedSignal(id);
  }

  return result;
}

ResultCode DataFile::setAccountName(i64 id, std::string name) {
  sqlite3_bind_text(stmt_setAccountName, 1, name.c_str(), static_cast<int>(name.length()), SQLITE_STATIC);
  sqlite3_bind_int64(stmt_setAccountName, 2, static_cast<sqlite3_int64>(id));
  sqlite3_step(stmt_setAccountName);
  sqlite3_clear_bindings(stmt_setAccountName); // Make sure that the char* binding is cleared
  auto result = mapSQLiteCodes(sqlite3_reset(stmt_setAccountName));
  if (result == ResultCode::OK) {
    accountUpdatedSignal(id);
  }
  return result;
}

ResultCode DataFile::setSecurityName(i64 id, std::string name) {
  sqlite3_bind_text(stmt_setSecurityName, 1, name.c_str(), static_cast<int>(name.length()), SQLITE_STATIC);
  sqlite3_bind_int64(stmt_setSecurityName, 2, static_cast<sqlite3_int64>(id));
  sqlite3_step(stmt_setSecurityName);
  sqlite3_clear_bindings(stmt_setSecurityName); // Make sure that the char* binding is cleared
  auto result = mapSQLiteCodes(sqlite3_reset(stmt_setSecurityName));
  if (result == ResultCode::OK) {
    securityUpdatedSignal(id);
  }
  return result;
}

ResultCode DataFile::setSecurityAssetClass(i64 id, std::string assetClass) {
  sqlite3_bind_text(stmt_setSecurityAssetClass, 1, assetClass.c_str(), static_cast<int>(assetClass.length()), SQLITE_STATIC);
  sqlite3_bind_int64(stmt_setSecurityAssetClass, 2, static_cast<sqlite3_int64>(id));
  sqlite3_step(stmt_setSecurityAssetClass);
  sqlite3_clear_bindings(stmt_setSecurityAssetClass); // Make sure that the char* binding is cleared
  auto result = mapSQLiteCodes(sqlite3_reset(stmt_setSecurityAssetClass));
  if (result == ResultCode::OK) {
    securityUpdatedSignal(id);
  }
  return result;
}

ResultCode DataFile::setSecuritySector(i64 id, std::string sector) {
  sqlite3_bind_text(stmt_setSecuritySector, 1, sector.c_str(), static_cast<int>(sector.length()), SQLITE_STATIC);
  sqlite3_bind_int64(stmt_setSecuritySector, 2, static_cast<sqlite3_int64>(id));
  sqlite3_step(stmt_setSecuritySector);
  sqlite3_clear_bindings(stmt_setSecuritySector); // Make sure that the char* binding is cleared
  auto result = mapSQLiteCodes(sqlite3_reset(stmt_setSecuritySector));
  if (result == ResultCode::OK) {
    securityUpdatedSignal(id);
  }
  return result;
}

ResultCode DataFile::addTransaction(i64 date, i64 account, Action action) noexcept {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_addTransaction, 1, static_cast<sqlite3_int64>(account));
  sqlite3_bind_int64(stmt_addTransaction, 2, static_cast<sqlite3_int64>(date));
  sqlite3_bind_int(stmt_addTransaction, 3, static_cast<int>(action));

  sqlite3_step(stmt_addTransaction);

  return mapSQLiteCodes(sqlite3_reset(stmt_addTransaction));
}

ResultCode DataFile::addBuyTransaction(i64 account, i64 date, i64 security, i64 numberOfShares, i64 sharePrice,
                                       i64 commission) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  beginSavepoint();

  auto result = addTransaction(date, account, Action::BUY);

  if (result != ResultCode::OK) {
    rollbackSavepoint();
    return result;
  }

  sqlite3_bind_int64(stmt_addBuyTransaction, 1, sqlite3_last_insert_rowid(db));
  sqlite3_bind_int64(stmt_addBuyTransaction, 2, static_cast<sqlite3_int64>(security));

  sqlite3_bind_int64(stmt_addBuyTransaction, 3, static_cast<sqlite3_int64>(numberOfShares));
  sqlite3_bind_int64(stmt_addBuyTransaction, 4, static_cast<sqlite3_int64>(sharePrice));
  sqlite3_bind_int64(stmt_addBuyTransaction, 5, static_cast<sqlite3_int64>(commission));
  result = mapSQLiteCodes(sqlite3_step(stmt_addBuyTransaction));
  sqlite3_reset(stmt_addBuyTransaction);

  if (result != ResultCode::OK) {
    rollbackSavepoint();
  } else {
    releaseSavepoint();
    transactionAddedSignal(lastInsertedId());
  }

  return result;
}

ResultCode DataFile::addSellTransaction(i64 account, i64 date, i64 security, i64 numberOfShares, i64 sharePrice,
                                        i64 commission) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  beginSavepoint();

  auto result = addTransaction(date, account, Action::SELL);

  if (result != ResultCode::OK) {
    rollbackSavepoint();
    return result;
  }

  sqlite3_bind_int64(stmt_addSellTransaction, 1, sqlite3_last_insert_rowid(db));
  sqlite3_bind_int64(stmt_addSellTransaction, 2, static_cast<sqlite3_int64>(security));

  sqlite3_bind_int64(stmt_addSellTransaction, 3, static_cast<sqlite3_int64>(numberOfShares));
  sqlite3_bind_int64(stmt_addSellTransaction, 4, static_cast<sqlite3_int64>(sharePrice));
  sqlite3_bind_int64(stmt_addSellTransaction, 5, static_cast<sqlite3_int64>(commission));
  result = mapSQLiteCodes(sqlite3_step(stmt_addSellTransaction));
  sqlite3_reset(stmt_addSellTransaction);

  if (result != ResultCode::OK) {
    rollbackSavepoint();
  } else {
    releaseSavepoint();
    transactionAddedSignal(lastInsertedId());
  }

  return result;
}

ResultCode DataFile::addDepositTransaction(i64 account, i64 date, std::optional<i64> security, i64 amount) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  beginSavepoint();

  auto result = addTransaction(date, account, Action::DEPOSIT);

  if (result != ResultCode::OK) {
    rollbackSavepoint();
    return result;
  }

  sqlite3_bind_int64(stmt_addDepositTransaction, 1, sqlite3_last_insert_rowid(db));
  if (security.has_value()) {
    sqlite3_bind_int64(stmt_addDepositTransaction, 2, static_cast<sqlite3_int64>(*security));
  } else {
    sqlite3_bind_null(stmt_addDepositTransaction, 2);
  }

  sqlite3_bind_int64(stmt_addDepositTransaction, 3, static_cast<sqlite3_int64>(amount));
  result = mapSQLiteCodes(sqlite3_step(stmt_addDepositTransaction));
  sqlite3_reset(stmt_addDepositTransaction);

  if (result != ResultCode::OK) {
    rollbackSavepoint();
  } else {
    releaseSavepoint();
    transactionAddedSignal(lastInsertedId());
  }

  return result;
}

ResultCode DataFile::addWithdrawTransaction(i64 account, i64 date, std::optional<i64> security, i64 amount) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  beginSavepoint();

  auto result = addTransaction(date, account, Action::WITHDRAW);

  if (result != ResultCode::OK) {
    rollbackSavepoint();
    return result;
  }

  sqlite3_bind_int64(stmt_addWithdrawTransaction, 1, sqlite3_last_insert_rowid(db));
  if (security.has_value()) {
    sqlite3_bind_int64(stmt_addWithdrawTransaction, 2, static_cast<sqlite3_int64>(*security));
  } else {
    sqlite3_bind_null(stmt_addWithdrawTransaction, 2);
  }

  sqlite3_bind_int64(stmt_addWithdrawTransaction, 3, static_cast<sqlite3_int64>(amount));
  result = mapSQLiteCodes(sqlite3_step(stmt_addWithdrawTransaction));
  sqlite3_reset(stmt_addWithdrawTransaction);

  if (result != ResultCode::OK) {
    releaseSavepoint();
  } else {
    releaseSavepoint();
    transactionAddedSignal(lastInsertedId());
  }

  return result;
}

ResultCode DataFile::addDividendTransaction(i64 account, i64 date, i64 security, i64 amount) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  beginSavepoint();

  auto result = addTransaction(date, account, Action::DIVIDEND);

  if (result != ResultCode::OK) {
    releaseSavepoint();
    return result;
  }

  sqlite3_bind_int64(stmt_addDividendTransaction, 1, sqlite3_last_insert_rowid(db));
  sqlite3_bind_int64(stmt_addDividendTransaction, 2, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt_addDividendTransaction, 3, static_cast<sqlite3_int64>(amount));
  result = mapSQLiteCodes(sqlite3_step(stmt_addDividendTransaction));
  sqlite3_reset(stmt_addDividendTransaction);

  if (result != ResultCode::OK) {
    rollbackSavepoint();
  } else {
    releaseSavepoint();
    transactionAddedSignal(lastInsertedId());
  }

  return result;
}

ResultCode DataFile::setBuyNumberOfShares(i64 transaction, i64 numberOfShares) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_setBuyNumberOfShares, 1, numberOfShares);
  sqlite3_bind_int64(stmt_setBuyNumberOfShares, 2, transaction);
  sqlite3_step(stmt_setBuyNumberOfShares);
  return finishTransactionUpdate(mapSQLiteCodes(sqlite3_reset(stmt_setBuyNumberOfShares)), transaction);
}

ResultCode DataFile::setBuySharePrice(i64 transaction, i64 sharePrice) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_setBuySharePrice, 1, static_cast<sqlite3_int64>(sharePrice));
  sqlite3_bind_int64(stmt_setBuySharePrice, 2, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(stmt_setBuySharePrice);
  return finishTransactionUpdate(mapSQLiteCodes(sqlite3_reset(stmt_setBuySharePrice)), transaction);
}

ResultCode DataFile::setBuyCommission(i64 transaction, i64 commission) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_setBuyCommission, 1, static_cast<sqlite3_int64>(commission));
  sqlite3_bind_int64(stmt_setBuyCommission, 2, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(stmt_setBuyCommission);
  return finishTransactionUpdate(mapSQLiteCodes(sqlite3_reset(stmt_setBuyCommission)), transaction);
}

ResultCode DataFile::setSellNumberOfShares(i64 transaction, i64 numberOfShares) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_setSellNumberOfShares, 1, static_cast<sqlite3_int64>(numberOfShares));
  sqlite3_bind_int64(stmt_setSellNumberOfShares, 2, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(stmt_setSellNumberOfShares);
  return finishTransactionUpdate(mapSQLiteCodes(sqlite3_reset(stmt_setSellNumberOfShares)), transaction);
}

ResultCode DataFile::setSellSharePrice(i64 transaction, i64 sharePrice) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_setSellSharePrice, 1, static_cast<sqlite3_int64>(sharePrice));
  sqlite3_bind_int64(stmt_setSellSharePrice, 2, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(stmt_setSellSharePrice);
  return finishTransactionUpdate(mapSQLiteCodes(sqlite3_reset(stmt_setSellSharePrice)), transaction);
}

ResultCode DataFile::setSellCommission(i64 transaction, i64 commission) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_setSellCommission, 1, static_cast<sqlite3_int64>(commission));
  sqlite3_bind_int64(stmt_setSellCommission, 2, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(stmt_setSellCommission);
  return finishTransactionUpdate(mapSQLiteCodes(sqlite3_reset(stmt_setSellCommission)), transaction);
}

ResultCode DataFile::setDepositAmount(i64 transaction, i64 amount) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_setDepositAmount, 1, static_cast<sqlite3_int64>(amount));
  sqlite3_bind_int64(stmt_setDepositAmount, 2, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(stmt_setDepositAmount);
  return finishTransactionUpdate(mapSQLiteCodes(sqlite3_reset(stmt_setDepositAmount)), transaction);
}

ResultCode DataFile::setWithdrawAmount(i64 transaction, i64 amount) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_setWithdrawAmount, 1, static_cast<sqlite3_int64>(amount));
  sqlite3_bind_int64(stmt_setWithdrawAmount, 2, static_cast<sqlite3_int64>(transaction));
  sqlite3_step(stmt_setWithdrawAmount);
  return finishTransactionUpdate(mapSQLiteCodes(sqlite3_reset(stmt_setWithdrawAmount)), transaction);
}

ResultCode DataFile::setDividendAmount(i64 transaction, i64 amount) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_setDividendAmount, 1, amount);
  sqlite3_bind_int64(stmt_setDividendAmount, 2, transaction);
  sqlite3_step(stmt_setDividendAmount);
  return finishTransactionUpdate(mapSQLiteCodes(sqlite3_reset(stmt_setDividendAmount)), transaction);
}

ResultCode DataFile::removeTransaction(i64 id) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_removeTransaction, 1, id);
  sqlite3_step(stmt_removeTransaction);
  auto result = mapSQLiteCodes(sqlite3_reset(stmt_removeTransaction));
  if (result == ResultCode::OK) {
    transactionRemovedSignal(id);
  }
  return result;
}

ResultCode DataFile::setSecurityPrice(i64 security, i64 date, i64 price) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_setSecurityPrice, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt_setSecurityPrice, 2, static_cast<sqlite3_int64>(date));
  sqlite3_bind_int64(stmt_setSecurityPrice, 3, static_cast<sqlite3_int64>(price));
  sqlite3_step(stmt_setSecurityPrice);
  auto result = mapSQLiteCodes(sqlite3_reset(stmt_setSecurityPrice));
  if (result == ResultCode::OK) {
    securityPriceUpdatedSignal(security, date);
  }
  return result;
}

ResultCode DataFile::removeSecurityPrice(i64 security, i64 date) {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_bind_int64(stmt_removeSecurityPrice, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt_removeSecurityPrice, 2, static_cast<sqlite3_int64>(date));
  sqlite3_step(stmt_removeSecurityPrice);
  auto result = mapSQLiteCodes(sqlite3_reset(stmt_removeSecurityPrice));
  if (result == ResultCode::OK) {
    securityPriceRemovedSignal(security, date);
  }
  return result;
}

i64 DataFile::lastInsertedId() const noexcept {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  return static_cast<i64>(sqlite3_last_insert_rowid(db));
}

ResultCode DataFile::beginTransaction() {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_step(stmt_beginTransaction);
  return mapSQLiteCodes(sqlite3_reset(stmt_beginTransaction));
}

ResultCode DataFile::rollbackTransaction() {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_step(stmt_rollbackTransaction);
  return mapSQLiteCodes(sqlite3_reset(stmt_rollbackTransaction));
}

ResultCode DataFile::commitTransaction() {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_step(stmt_commitTransaction);
  return mapSQLiteCodes(sqlite3_reset(stmt_commitTransaction));
}

ResultCode DataFile::copyTo(DataFile& other) const noexcept {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");

  sqlite3_backup* backup = sqlite3_backup_init(other.db, "main", db, "main");

  if (backup) {
    sqlite3_backup_step(backup, -1);
    sqlite3_backup_finish(backup);
  }

  return mapSQLiteCodes(sqlite3_errcode(other.db));
}

bool DataFile::hasTransaction() const noexcept {
  assert(db != nullptr && "Using DataFile in invalid state, most likely a use-after-move");
  return sqlite3_get_autocommit(db) == 0;
}

std::optional<std::string> DataFile::filePath() const noexcept {
  const char* cStr = sqlite3_db_filename(db, "main");
  if (cStr == nullptr || cStr[0] == '\0') {
    // This is an in-memory or temp database
    return std::nullopt;
  }
  return std::string(cStr);
}

const char* pv::DataFile::errMsg() const noexcept { return sqlite3_errmsg(db); }

Connection DataFile::onChanged(const ChangedSignal::slot_type& slot) {
  return changedSignal.connect(slot);
}

Connection DataFile::onAccountAdded(const AccountAddedSignal::slot_type& slot) {
  return accountAddedSignal.connect(slot);
}

Connection DataFile::onAccountUpdated(const AccountUpdatedSignal::slot_type& slot) {
  return accountUpdatedSignal.connect(slot);
}

Connection DataFile::onAccountRemoved(const AccountRemovedSignal::slot_type& slot) {
  return accountRemovedSignal.connect(slot);
}

Connection DataFile::onTransactionAdded(const TransactionAddedSignal::slot_type& slot) {
  return transactionAddedSignal.connect(slot);
}

Connection DataFile::onTransactionUpdated(const TransactionUpdatedSignal::slot_type& slot) {
  return transactionUpdatedSignal.connect(slot);
}

Connection DataFile::onTransactionRemoved(const TransactionRemovedSignal::slot_type& slot) {
  return transactionRemovedSignal.connect(slot);
}

Connection DataFile::onSecurityAdded(const SecurityAddedSignal::slot_type& slot) {
  return securityAddedSignal.connect(slot);
}

Connection DataFile::onSecurityUpdated(const SecurityUpdatedSignal::slot_type& slot) {
  return securityUpdatedSignal.connect(slot);
}

Connection DataFile::onSecurityRemoved(const SecurityRemovedSignal::slot_type& slot) {
  return securityRemovedSignal.connect(slot);
}

Connection DataFile::onSecurityPriceUpdated(const SecurityPriceUpdatedSignal::slot_type& slot) {
  return securityPriceUpdatedSignal.connect(slot);
}

Connection DataFile::onSecurityPriceRemoved(const SecurityPriceRemovedSignal::slot_type& slot) {
  return securityPriceRemovedSignal.connect(slot);
}

Connection DataFile::onRollback(const RollbackSignal::slot_type& slot) { return rollbackSignal.connect(slot); }

} // namespace pv

