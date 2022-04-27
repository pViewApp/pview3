#include "Account.h"
#include "Action.h"
#include "Security.h"
#include "Transaction.h"
#include <atomic>

namespace pv {

// Impl
class Account::Shared {
private:
  friend class Account;

  std::atomic_bool valid = true;

  DataFile* dataFile;
  unsigned int id;
  std::string name;

  std::atomic_uint nextTransactionId = 0;
  std::vector<Transaction> transactions;

  mutable boost::signals2::signal<void(std::string, std::string)> signal_nameChanged;

  mutable boost::signals2::signal<void(Transaction&)> signal_transactionAdded;
  mutable boost::signals2::signal<void(const Transaction&)> signal_transactionRemoved;

  mutable boost::signals2::signal<void()> signal_invalidated;

  // Implemented here because we need to call in destructor
  bool invalidate() noexcept {
    bool wasValid = valid.exchange(false);
    if (!wasValid) {
      return false;
    }

    signal_invalidated();
    return wasValid;
  }

public:
  Shared(DataFile& dataFile, unsigned int id, std::string name) noexcept : dataFile(&dataFile), id(id), name(name) {}
  ~Shared() { invalidate(); }
};

// Account
// not Impl
Account::Account(DataFile& dataFile, unsigned int id, std::string name) noexcept
    : shared(std::make_shared<Account::Shared>(dataFile, id, name)) {}

bool Account::invalidate() noexcept { return shared->invalidate(); }

const DataFile* Account::dataFile() const noexcept { return shared->dataFile; }

DataFile* Account::dataFile() noexcept { return shared->dataFile; }

unsigned int Account::id() const noexcept { return shared->id; }

std::string Account::name() const noexcept { return shared->name; }

const std::vector<Transaction>& Account::transactions() const noexcept { return shared->transactions; }

bool Account::valid() const noexcept { return shared->valid; }

bool Account::setName(std::string name) noexcept {
  std::string oldName = shared->name;
  shared->name = name;
  shared->signal_nameChanged(shared->name, oldName);
  return true;
}

std::variant<Transaction, TransactionAdditionError>
Account::addTransaction(Date date, const Action& action, std::optional<const Security> security, Decimal numberOfShares,
                        Decimal sharePrice, Decimal commission, Decimal totalAmount) noexcept {
  if (!valid())
    return TransactionAdditionError::INVALID_ACCOUNT;
  if (security.has_value()) {
    // Ensure security is valid
    if (!security->valid())
      return TransactionAdditionError::INVALID_SECURITY;
    if (security->dataFile() != dataFile())
      return TransactionAdditionError::INVALID_SECURITY;
  }

  action.processTransactionParamaters(date, security, numberOfShares, sharePrice, commission, totalAmount);

  auto transaction = pv::Transaction(*this, shared->nextTransactionId.fetch_add(1), date, action, security,
                                     numberOfShares, sharePrice, commission, totalAmount);

  shared->transactions.push_back(transaction);

  shared->signal_transactionAdded(transaction);

  return transaction;
}

boost::signals2::signal<void(std::string, std::string)>& Account::nameChanged() const noexcept {
  return shared->signal_nameChanged;
}

boost::signals2::signal<void(Transaction&)>& Account::transactionAdded() const noexcept {
  return shared->signal_transactionAdded;
}

boost::signals2::signal<void(const Transaction&)>& Account::transactionRemoved() const noexcept {
  return shared->signal_transactionRemoved;
}

boost::signals2::signal<void()>& Account::invalidated() const noexcept { return shared->signal_invalidated; }

} // namespace pv
