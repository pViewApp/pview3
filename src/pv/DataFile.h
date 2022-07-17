#ifndef PV_DATAFILE_H
#define PV_DATAFILE_H

#include "Integer64.h"
#include "Signals.h"
#include <string>
#include <optional>
#include <memory>
#include <unordered_map>

class sqlite3;
class sqlite3_stmt;

namespace pv {

enum class Action : unsigned char {
  BUY = 0,
  SELL = 1,
  DEPOSIT = 2,
  WITHDRAW = 3,
  DIVIDEND = 4,
  INTEREST = 5,
};

// Codes beginning with SQL_ are forwarded from SQLite.
enum class ResultCode : unsigned char {
  OK = 0,

  SQL_ERROR = 1,
  SQL_NOMEM = 2,
  SQL_CORRUPT = 4,
  SQL_CONSTRAINT = 5,

  MODIFICATION_PROHIBITED = 32,
  /// \brief An error code indicating that the action could not
  /// be completed because the record could not be found (for example,
  /// deleting a non-existent transaction).
  ///
  /// This result code indicates that no change has occured.
  RECORD_NOT_FOUND = 33,
};

using StatementPointer = std::unique_ptr<sqlite3_stmt, int(*)(sqlite3_stmt*)>;

class DataFile {
public:
  using ChangedSignal = Signal<>;

  using AccountAddedSignal = Signal<i64>;
  using AccountUpdatedSignal = Signal<i64>;
  using AccountRemovedSignal = Signal<i64>;

  using TransactionAddedSignal = Signal<i64>;
  using TransactionUpdatedSignal = Signal<i64>;
  using TransactionRemovedSignal = Signal<i64>;

  using SecurityAddedSignal = Signal<i64>;
  using SecurityUpdatedSignal = Signal<i64>;
  using SecurityRemovedSignal = Signal<i64>;

  using SecurityPriceUpdatedSignal = Signal<i64, i64>;
  using SecurityPriceRemovedSignal = Signal<i64, i64>;

  using RollbackSignal = Signal<>;
private:
  sqlite3_stmt* prepare(std::string sql, int flags = 0, ResultCode* outResult = nullptr) noexcept;

  // pView transactions
  /// \internal Adds a pView (not SQL!) transaciton to the master transaction table.
  /// Make sure to add a row to the corresponding action-specific
  /// transaction table after calling this function.
  ResultCode addTransaction(i64 date, i64 account, Action action) noexcept;

  /// \internal
  /// If \c code is not OK, return code. Otherwise, if one row was changed in the database,
  /// return OK. Otherwise, return RECORD_NOT_FOUND. This also emits transactionUpdatedSignal if the edit was successful.
  /// This is intended to be used at the end of the routines for modifying transactions
  /// (setBuyNumberOfShares, setDividendAmount, etc.).
  ResultCode finishTransactionUpdate(ResultCode code, pv::i64 transaction) noexcept;

  /// \internal Use these instead of beginTransaction() for internal code, because
  /// this can be nested within transactions created by the user.
  ResultCode beginSavepoint();
  ResultCode rollbackSavepoint();
  ResultCode releaseSavepoint();

  //// FIELDS GO HERE
  //// REMEMBER TO UPDATE THE DESTRUCTOR AND swap() FUNCTION
  ChangedSignal changedSignal;

  AccountAddedSignal accountAddedSignal;
  AccountUpdatedSignal accountUpdatedSignal;
  AccountRemovedSignal accountRemovedSignal;

  TransactionAddedSignal transactionAddedSignal;
  TransactionUpdatedSignal transactionUpdatedSignal;
  TransactionRemovedSignal transactionRemovedSignal;

  SecurityAddedSignal securityAddedSignal;
  SecurityUpdatedSignal securityUpdatedSignal;
  SecurityRemovedSignal securityRemovedSignal;

  SecurityPriceUpdatedSignal securityPriceUpdatedSignal;
  SecurityPriceRemovedSignal securityPriceRemovedSignal;

  void updateSQLiteHooks();

  /// \internal
  /// Emit whenever a transaction (not savepoint) is rolled back, since any changes
  /// made within will be lost.
  ///
  /// For savepoints, do NOT emit this. We must still keep track of the state during save points.

  bool suppressRollbackSignal = false;
  RollbackSignal rollbackSignal;

  sqlite3* db = nullptr;

  sqlite3_stmt* stmt_addAccount = nullptr;
  sqlite3_stmt* stmt_addSecurity = nullptr;

  sqlite3_stmt* stmt_removeAccount = nullptr;
  sqlite3_stmt* stmt_removeSecurity = nullptr;

  sqlite3_stmt* stmt_setAccountName = nullptr;

  sqlite3_stmt* stmt_setSecurityName = nullptr;
  sqlite3_stmt* stmt_setSecurityAssetClass = nullptr;
  sqlite3_stmt* stmt_setSecuritySector = nullptr;

  sqlite3_stmt* stmt_addTransaction = nullptr;

  sqlite3_stmt* stmt_addBuyTransaction = nullptr;
  sqlite3_stmt* stmt_addSellTransaction = nullptr;
  sqlite3_stmt* stmt_addDepositTransaction = nullptr;
  sqlite3_stmt* stmt_addWithdrawTransaction = nullptr;
  sqlite3_stmt* stmt_addDividendTransaction = nullptr;
  sqlite3_stmt* stmt_addInterestTransaction = nullptr;

  sqlite3_stmt* stmt_setBuyNumberOfShares = nullptr;
  sqlite3_stmt* stmt_setBuySharePrice = nullptr;
  sqlite3_stmt* stmt_setBuyCommission = nullptr;
  sqlite3_stmt* stmt_setSellNumberOfShares = nullptr;
  sqlite3_stmt* stmt_setSellSharePrice = nullptr;
  sqlite3_stmt* stmt_setSellCommission = nullptr;
  sqlite3_stmt* stmt_setDepositAmount = nullptr;
  sqlite3_stmt* stmt_setWithdrawAmount = nullptr;
  sqlite3_stmt* stmt_setDividendAmount = nullptr;
  sqlite3_stmt* stmt_setInterestAmount = nullptr;

  sqlite3_stmt* stmt_removeTransaction = nullptr;

  sqlite3_stmt* stmt_setSecurityPrice = nullptr;
  sqlite3_stmt* stmt_removeSecurityPrice = nullptr;

  sqlite3_stmt* stmt_beginTransaction = nullptr;
  sqlite3_stmt* stmt_rollbackTransaction = nullptr;
  sqlite3_stmt* stmt_commitTransaction = nullptr;
  sqlite3_stmt* stmt_vacuumInto = nullptr;

  sqlite3_stmt* stmt_beginSavepoint = nullptr;
  sqlite3_stmt* stmt_rollbackSavepoint = nullptr;
  sqlite3_stmt* stmt_releaseSavepoint = nullptr;

  std::unordered_map<const char*, sqlite3_stmt*> queryCache;
public:
  explicit DataFile(std::string location = ":memory:", int flags = -1);

  DataFile(const DataFile&) = delete;
  DataFile& operator=(const DataFile&) = delete;

  DataFile(DataFile&& other) noexcept;
  DataFile& operator=(DataFile&& other) noexcept;
  ~DataFile() noexcept;

  StatementPointer query(std::string query) const noexcept;

  /// \brief Prepares an SQL query to be executed against the database.
  /// 
  /// The query must be a read-only operation. Undefined behaviour occurs if it tries
  /// to modify the database.
  ///
  /// The returned \c sqlite3_stmt* must not be manually finalized up by the user.
  ///
  /// \param query the SQL query text
  /// \param result Output: a \c ResultCode that gives the results of this operation (use \c nullptr to ignore)
  /// \return an SQLite prepared statement, or nullptr if an error occured
  sqlite3_stmt* cachedQuery(const char* query) noexcept;

  ResultCode addAccount(std::string name);
  ResultCode addSecurity(std::string symbol, std::string name, std::string assetClass, std::string sector);

  ResultCode removeAccount(i64 id);
  ResultCode removeSecurity(i64 id);

  ResultCode setAccountName(i64 id, std::string name);

  ResultCode setSecurityName(i64 id, std::string name);
  ResultCode setSecurityAssetClass(i64 id, std::string assetClass);
  ResultCode setSecuritySector(i64 id, std::string sector);

  ResultCode addBuyTransaction(i64 account, i64 date, i64 security, i64 numberOfShares, i64 sharePrice, i64 commission);
  ResultCode addSellTransaction(i64 account, i64 date, i64 security, i64 numberOfShares, i64 sharePrice, i64 commission);
  ResultCode addDepositTransaction(i64 account, i64 date, std::optional<i64> security, i64 amount);
  ResultCode addWithdrawTransaction(i64 account, i64 date, std::optional<i64> security, i64 amount);
  ResultCode addDividendTransaction(i64 account, i64 date, i64 security, i64 amount);
  ResultCode addInterestTransaction(i64 account, i64 date, i64 security, i64 amount);

  ResultCode removeTransaction(i64 id);

  ResultCode setBuyNumberOfShares(i64 transaction, i64 numberOfShares);
  ResultCode setBuySharePrice(i64 transaction, i64 sharePrice);
  ResultCode setBuyCommission(i64 transaction, i64 commission);

  ResultCode setSellNumberOfShares(i64 transaction, i64 numberOfShares);
  ResultCode setSellSharePrice(i64 transaction, i64 sharePrice);
  ResultCode setSellCommission(i64 transaction, i64 commission);

  ResultCode setDepositAmount(i64 transaction, i64 amount);

  ResultCode setWithdrawAmount(i64 transaction, i64 amount);
  
  ResultCode setDividendAmount(i64 transaction, i64 amount);

  ResultCode setInterestAmount(i64 transaction, i64 amount);

  ResultCode setSecurityPrice(i64 security, i64 date, i64 price);
  ResultCode removeSecurityPrice(i64 security, i64 date);

  /// \brief Gets the id of the last inserted account/security/transaction.
  ///
  /// If another modification has occured since the last call to \c addAccount, \c addSecurity,
  /// or any of the \c add*Transaction routines, then this function's return value is undefined.
  ///
  /// \returns the last inserted id
  i64 lastInsertedId() const noexcept;

  /// \brief Begins a transaction (in the SQL sense).
  /// 
  /// This cannot be nested.
  ResultCode beginTransaction();
  /// \brief Rolls back the current transaction.
  ResultCode rollbackTransaction();
  /// \brief Commits the current transaction.
  ResultCode commitTransaction();

  ResultCode copyTo(DataFile& other) const noexcept;

  bool hasTransaction() const noexcept;

  std::optional<std::string> filePath() const noexcept; 

  const char* errMsg() const noexcept; 

  Connection onChanged(const ChangedSignal::slot_type& slot);

  Connection onAccountAdded(const AccountAddedSignal::slot_type& slot);
  Connection onAccountUpdated(const AccountUpdatedSignal::slot_type& slot);
  Connection onAccountRemoved(const AccountRemovedSignal::slot_type& slot);

  Connection onTransactionAdded(const TransactionAddedSignal::slot_type& slot);
  Connection onTransactionUpdated(const TransactionUpdatedSignal::slot_type& slot);
  Connection onTransactionRemoved(const TransactionRemovedSignal::slot_type& slot);

  Connection onSecurityAdded(const SecurityAddedSignal::slot_type& slot);
  Connection onSecurityUpdated(const SecurityUpdatedSignal::slot_type& slot);
  Connection onSecurityRemoved(const SecurityRemovedSignal::slot_type& slot);

  Connection onSecurityPriceUpdated(const SecurityPriceUpdatedSignal::slot_type& slot);
  Connection onSecurityPriceRemoved(const SecurityPriceRemovedSignal::slot_type& slot);

  Connection onRollback(const RollbackSignal::slot_type& slot);

  friend void swap(DataFile& lhs, DataFile& rhs) noexcept;
};

void swap(DataFile& lhs, DataFile& rhs) noexcept;

} // namespace pv

#endif // PV_DATAFILE_H
