#include "Account.h"
#include "DataFile.h"
#include "Security.h"
#include "Transaction.h"
#include <unordered_map>

namespace pv {

Account::Account(const DataFile& dataFile, std::string name) : dataFile_(&dataFile), name_(name) {}

Account::~Account() {
  for (auto* t : transactions_) {
    delete t;
  }
}

const DataFile& Account::dataFile() const noexcept { return *dataFile_; }

std::string Account::name() const noexcept { return name_; }

const std::vector<const Transaction*>& Account::transactions() const noexcept { return transactions_; }

void Account::setName(std::string name) noexcept {
  std::string oldName = std::move(name_);
  name_ = name;
  signal_nameChanged(std::move(name), oldName);
}

#define REPLACE_TRANSACTION(index, transaction)                                                                        \
  if (transactions_.size() <= index) {                                                                                 \
    return TransactionOperationResult::INVALID_TRANSACTION;                                                            \
  }                                                                                                                    \
                                                                                                                       \
  auto* oldTransaction = transactions_[index];                                                                         \
  auto* newTransaction = new (decltype(transaction))(std::move(transaction));                                          \
  transactions_[index] = newTransaction;                                                                               \
  signal_transactionReplaced(index, newTransaction, oldTransaction);                                                   \
  delete oldTransaction;                                                                                               \
                                                                                                                       \
  return TransactionOperationResult::SUCCESS;
// end define

TransactionOperationResult Account::replaceTransaction(size_t index, BuyTransaction transaction) {
  REPLACE_TRANSACTION(index, transaction);
}

TransactionOperationResult Account::replaceTransaction(size_t index, SellTransaction transaction) {
  REPLACE_TRANSACTION(index, transaction);
}

TransactionOperationResult Account::replaceTransaction(size_t index, DepositTransaction transaction) {
  REPLACE_TRANSACTION(index, transaction);
}

TransactionOperationResult Account::replaceTransaction(size_t index, WithdrawTransaction transaction) {
  REPLACE_TRANSACTION(index, transaction);
}

TransactionOperationResult Account::replaceTransaction(size_t index, DividendTransaction transaction) {
  REPLACE_TRANSACTION(index, transaction);
}

#undef REPLACE_TRANSACTION

#define ADD_TRANSACTION(transaction)                                                                                   \
  if (transaction.security != nullptr && !dataFile_->owns(*transaction.security)) {                                    \
    return TransactionOperationResult::INVALID_SECURITY;                                                               \
  }                                                                                                                    \
                                                                                                                       \
  auto index = transactions_.size();                                                                                   \
  signal_beforeTransactionAdded(index);                                                                                \
                                                                                                                       \
  auto* t = new (decltype(transaction))(std::move(transaction));                                                       \
                                                                                                                       \
  try {                                                                                                                \
    transactions_.push_back(t);                                                                                        \
  } catch (...) {                                                                                                      \
    delete t;                                                                                                          \
    return TransactionOperationResult::OTHER;                                                                          \
  }                                                                                                                    \
                                                                                                                       \
  signal_transactionAdded(index, t);                                                                                   \
                                                                                                                       \
  return TransactionOperationResult::SUCCESS;
// end define

TransactionOperationResult Account::addTransaction(BuyTransaction transaction) { ADD_TRANSACTION(transaction); }

TransactionOperationResult Account::addTransaction(SellTransaction transaction) { ADD_TRANSACTION(transaction); }

TransactionOperationResult Account::addTransaction(DepositTransaction transaction) { ADD_TRANSACTION(transaction); }

TransactionOperationResult Account::addTransaction(WithdrawTransaction transaction) { ADD_TRANSACTION(transaction); }

TransactionOperationResult Account::addTransaction(DividendTransaction transaction) { ADD_TRANSACTION(transaction); }

#undef ADD_TRANSACTION

TransactionOperationResult Account::removeTransaction(std::size_t index) noexcept {
  if (transactions_.size() <= index) {
    return TransactionOperationResult::INVALID_TRANSACTION; // Out of bounds
  }

  auto iter = transactions_.begin() + index;
  signal_beforeTransactionRemoved(index, *iter);
  transactions_.erase(iter);
  signal_transactionRemoved(index);

  return TransactionOperationResult::SUCCESS;
}

Connection Account::listenNameChanged(const NameChangedSignal::slot_type& slot) noexcept {
  return signal_nameChanged.connect(slot);
}

Connection Account::listenBeforeTransactionAdded(const BeforeTransactionAddedSignal::slot_type& slot) noexcept {
  return signal_beforeTransactionAdded.connect(slot);
}

Connection Account::listenTransactionAdded(const TransactionAddedSignal::slot_type& slot) noexcept {
  return signal_transactionAdded.connect(slot);
}

Connection Account::listenBeforeTransactionRemoved(const BeforeTransactionRemovedSignal::slot_type& slot) noexcept {
  return signal_beforeTransactionRemoved.connect(slot);
}

Connection Account::listenTransactionRemoved(const TransactionRemovedSignal::slot_type& slot) noexcept {
  return signal_transactionRemoved.connect(slot);
}

Connection Account::listenTransactionReplaced(const TransactionReplacedSignal::slot_type& slot) noexcept {
  return signal_transactionReplaced.connect(slot);
}

} // namespace pv
