#ifndef PV_TRANSACTION_H
#define PV_TRANSACTION_H

#include "Date.h"
#include "Decimal.h"
#include "Invalidatable.h"
#include "Security.h"
#include <boost/signals2/signal.hpp>
#include <functional>
#include <memory>
#include <optional>

namespace pv {

class Account;
class Action;
class DataFile;
class Security;

enum class TransactionEditResult {
  Success = 0,
  UnknownFailure = 1,
  InvalidTransaction = 2,
};

class Transaction : public Invalidatable {
private:
  class Shared;
  friend class Account;
  friend struct std::hash<Transaction>; // Let hasher access the pointer

  std::shared_ptr<Shared> shared;

  // pv::Account is responsible for proccessing paramaters to the constructor
  Transaction(Account& account, unsigned int id, Date date, const Action& action,
              std::optional<const Security> security, Decimal numberOfShares, Decimal sharePrice, Decimal commission,
              Decimal totalAmount);

  bool invalidate() noexcept;

public:
  Transaction(const Transaction&) = default;
  Transaction& operator=(const Transaction&) = default;

  // Getters
  const Account* account() const noexcept;
  Account* account() noexcept;
  unsigned int id() const noexcept;
  Date date() const noexcept;
  const Action& action() const noexcept;
  const std::optional<Security> security() const noexcept;
  Decimal numberOfShares() const noexcept;
  Decimal sharePrice() const noexcept;
  Decimal commission() const noexcept;
  Decimal totalAmount() const noexcept;

  bool valid() const noexcept;

  // Mutators

  TransactionEditResult setNumberOfShares(Decimal numberOfShares) noexcept;
  TransactionEditResult setSharePrice(Decimal sharePrice) noexcept;
  TransactionEditResult setCommission(Decimal commission) noexcept;
  TransactionEditResult setTotalAmount(Decimal totalAmount) noexcept;

  // Signals

  boost::signals2::signal<void(const Decimal&, const Decimal&)>& numberOfSharesChanged() const noexcept;
  boost::signals2::signal<void(const Decimal&, const Decimal&)>& sharePriceChanged() const noexcept;
  boost::signals2::signal<void(const Decimal&, const Decimal&)>& commissionChanged() const noexcept;
  boost::signals2::signal<void(const Decimal&, const Decimal&)>& totalAmountChanged() const noexcept;
  boost::signals2::signal<void()>& changed() const noexcept;

  boost::signals2::signal<void()>& invalidated() const noexcept override;

  // Operators
  bool operator==(const Transaction& other) const noexcept { return shared == other.shared; }
  bool operator!=(const Transaction& other) const noexcept { return shared != other.shared; }
};

} // namespace pv

template <> struct std::hash<pv::Transaction> {
public:
  std::size_t operator()(const pv::Transaction& transaction) const noexcept { return sharedHasher(transaction.shared); }

private:
  inline static const std::hash<std::shared_ptr<pv::Transaction::Shared>> sharedHasher;
};

#endif // PV_TRANSACTION_H
