#ifndef PV_ACCOUNT_H
#define PV_ACCOUNT_H

#include "Date.h"
#include "Decimal.h"
#include "Invalidatable.h"
#include "Result.h"
#include "Signals.h"
#include "Transaction.h"
#include <boost/signals2/signal.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace pv {

class DataFile;

enum class TransactionOperationResult : unsigned short {
  SUCCESS = 0,
  OTHER = 1,
  INVALID_SECURITY = 2,
  INVALID_TRANSACTION = 3,
};

class Account {
public:
  using NameChangedSignal = Signal<void(std::string, std::string)>;

  using BeforeTransactionAddedSignal = Signal<void(std::size_t)>;
  using TransactionAddedSignal = Signal<void(std::size_t, const Transaction*)>;

  using BeforeTransactionRemovedSignal = Signal<void(std::size_t, const Transaction*)>;
  using TransactionRemovedSignal = Signal<void(std::size_t)>;

  using TransactionReplacedSignal = Signal<void(std::size_t, const Transaction*, const Transaction*)>;

private:
  /// \internal
  /// This is a pointer instead of a reference to get the compiler-generated move constructor, but
  /// treat this as if it were a reference.
  const DataFile* dataFile_;
  std::string name_;
  std::vector<const Transaction*> transactions_;

  NameChangedSignal signal_nameChanged;

  BeforeTransactionAddedSignal signal_beforeTransactionAdded;
  TransactionAddedSignal signal_transactionAdded;

  BeforeTransactionRemovedSignal signal_beforeTransactionRemoved;
  TransactionRemovedSignal signal_transactionRemoved;
  TransactionReplacedSignal signal_transactionReplaced;

public:
  Account(const DataFile& dataFile, std::string name);
  Account(Account&& other) = default;
  Account& operator=(Account&& other) = default;

  ~Account();

  // GETTERS
  const DataFile& dataFile() const noexcept;
  std::string name() const noexcept;
  const std::vector<const Transaction*>& transactions() const noexcept;

  /// \brief Set's the name of this Account.
  void setName(std::string name) noexcept;

  TransactionOperationResult replaceTransaction(std::size_t index, BuyTransaction transaction);
  TransactionOperationResult replaceTransaction(std::size_t index, SellTransaction transaction);
  TransactionOperationResult replaceTransaction(std::size_t index, DepositTransaction transaction);
  TransactionOperationResult replaceTransaction(std::size_t index, WithdrawTransaction transaction);
  TransactionOperationResult replaceTransaction(std::size_t index, DividendTransaction transaction);

  TransactionOperationResult addTransaction(BuyTransaction transaction);
  TransactionOperationResult addTransaction(SellTransaction transaction);
  TransactionOperationResult addTransaction(DepositTransaction transaction);
  TransactionOperationResult addTransaction(WithdrawTransaction transaction);
  TransactionOperationResult addTransaction(DividendTransaction transaction);

  TransactionOperationResult removeTransaction(std::size_t index) noexcept;

  Connection listenNameChanged(const NameChangedSignal::slot_type& slot) noexcept;

  Connection listenBeforeTransactionAdded(const BeforeTransactionAddedSignal::slot_type& slot) noexcept;
  Connection listenTransactionAdded(const TransactionAddedSignal::slot_type& slot) noexcept;

  Connection listenBeforeTransactionRemoved(const BeforeTransactionRemovedSignal::slot_type& slot) noexcept;
  Connection listenTransactionRemoved(const TransactionRemovedSignal::slot_type& slot) noexcept;
  Connection listenTransactionReplaced(const TransactionReplacedSignal::slot_type& slot) noexcept;
};

} // namespace pv
#endif // PV_ACCOUNT_H
