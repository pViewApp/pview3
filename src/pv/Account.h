#ifndef PV_ACCOUNT_H
#define PV_ACCOUNT_H

#include "Date.h"
#include "Decimal.h"
#include "Invalidatable.h"
#include <boost/signals2/signal.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace pv {

class Action;
class DataFile;
class Transaction;
class Security;

enum class TransactionAdditionError {
  UNKNOWN = 0,
  INVALID_ACCOUNT = 1,
  INVALID_SECURITY = 2,
};

class Account : public Invalidatable {
private:
  class Shared;
  friend class DataFile;
  friend struct std::hash<Account>;

  std::shared_ptr<Shared> shared;

  Account(DataFile& dataFile, unsigned int id, std::string name) noexcept;
  bool invalidate() noexcept;

public:
  Account(const pv::Account& other) = default;
  Account& operator=(const pv::Account&) = default;

  // getters
  const DataFile* dataFile() const noexcept;
  unsigned int id() const noexcept;
  std::string name() const noexcept;
  const std::vector<Transaction>& transactions() const noexcept;

  DataFile* dataFile() noexcept;

  bool valid() const noexcept;

  // mutators
  bool setName(std::string name) noexcept;

  std::variant<Transaction, TransactionAdditionError> addTransaction(Date date, const Action& action,
                                                                     std::optional<const Security> security,
                                                                     Decimal numberOfShares, Decimal sharePrice,
                                                                     Decimal commission, Decimal totalAmount) noexcept;

  bool removeTransaction(Transaction transaction) noexcept;

  // Internal

  /// \internal
  /// Todo, implement in future.
  int requestTransactionChange_(const Transaction& t, const Date& date, const Action& action,
                                const std::optional<const pv::Security>& security, const Decimal& numberOfShares,
                                const Decimal& sharePrice, const Decimal& commission, const Decimal& totalAmount);

  // signals

  /// \brief Fired whenever the name changes
  ///
  /// # Signal Arguments
  /// \arg \c 1 The new name
  /// \arg \c 2 The old name
  boost::signals2::signal<void(std::string, std::string)>& nameChanged() const noexcept;
  /// \brief Fired whenever a transaction is added
  ///
  /// # Signal Arguments
  /// \arg \c 1 The added transaction
  boost::signals2::signal<void(Transaction&)>& transactionAdded() const noexcept;
  /// \brief Fired whenever a transaction is removed
  ///
  /// #Signal Arguments
  /// \arg \c 1 The removed transaction
  boost::signals2::signal<void(const Transaction&)>& transactionRemoved() const noexcept;
  /// \brief Fired whenever a transaction is changed
  ///
  /// Equivalent to pv::Transaction::changed().
  ///
  /// #Signal Arguments
  /// \arg \c 1 The changed transaction
  boost::signals2::signal<void(const Transaction&)>& transactionChanged() const noexcept;
  /// \brief Fired when this account becomes invalid.
  ///
  /// #Signal Arguments
  /// \arg \c 1 \c this
  boost::signals2::signal<void()>& invalidated() const noexcept override;

  // operators

  bool operator==(const Account& other) const noexcept { return shared == other.shared; }
  bool operator!=(const Account& other) const noexcept { return shared != other.shared; }

  bool operator<(const Account& other) const noexcept;

  bool operator>(const Account& other) const noexcept;
};

} // namespace pv

template <> struct std::hash<pv::Account> {
public:
  std::size_t operator()(const pv::Account& account) const noexcept { return sharedHasher(account.shared); }

private:
  inline static const std::hash<std::shared_ptr<pv::Account::Shared>> sharedHasher;
};

#endif // PV_ACCOUNT_H
