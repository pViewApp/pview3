#ifndef PV_TRANSACTION_H
#define PV_TRANSACTION_H

#include "Date.h"
#include "Decimal.h"
#include "Security.h"
#include <memory>
#include <optional>

namespace pv {

class Account;
class Action;
class DataFile;
class Security;

class Transaction {
private:
  class Shared;
  friend class Account;

  std::shared_ptr<Shared> shared;

  // pv::Account is responsible for proccessing paramaters to the constructor
  Transaction(Account& account, unsigned int id, Date date, const Action& action,
              std::optional<const Security> security, Decimal numberOfShares, Decimal sharePrice, Decimal commission,
              Decimal totalAmount);

public:
  Transaction(const Transaction&) = default;
  Transaction& operator=(const Transaction&) = default;

  // Getters
  const Account& account() const noexcept;
  Account& account() noexcept;
  unsigned int id() const noexcept;
  Date date() const noexcept;
  const Action& action() const noexcept;
  const std::optional<Security> security() const noexcept;
  std::optional<Security> security() noexcept;
  Decimal numberOfShares() const noexcept;
  Decimal sharePrice() const noexcept;
  Decimal commission() const noexcept;
  Decimal totalAmount() const noexcept;

  bool valid();

  // Operators
  bool operator==(const Transaction& other) const noexcept { return shared == other.shared; }
};

} // namespace pv

#endif // PV_TRANSACTION_H
