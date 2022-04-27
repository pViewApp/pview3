#include "Transaction.h"
#include <atomic>

namespace pv {

class Transaction::Shared {
private:
  friend class Transaction;

  Account* account;
  unsigned int id;
  Date date;
  const Action& action;
  std::optional<const Security> security;
  Decimal numberOfShares;
  Decimal sharePrice;
  Decimal commission;
  Decimal totalAmount;

  std::atomic_bool valid = true;

public:
  Shared(Account& account, unsigned int id, Date date, const Action& action, std::optional<const Security> security,
         Decimal numberOfShares, Decimal sharePrice, Decimal commission, Decimal totalAmount)
      : account(&account), id(id), date(date), action(action), security(security), numberOfShares(numberOfShares),
        sharePrice(sharePrice), commission(commission), totalAmount(totalAmount) {}
};

Transaction::Transaction(Account& account, unsigned int id, Date date, const Action& action,
                         std::optional<const Security> security, Decimal numberOfShares, Decimal sharePrice,
                         Decimal commission, Decimal totalAmount)
    : shared(std::make_shared<Transaction::Shared>(account, id, date, action, security, numberOfShares, sharePrice,
                                                   commission, totalAmount)) {}

const Account& Transaction::account() const noexcept {
  return *shared->account; // todo unsafe
}

Account& Transaction::account() noexcept { return *shared->account; }

unsigned int Transaction::id() const noexcept { return shared->id; }

Date Transaction::date() const noexcept { return shared->date; }

const Action& Transaction::action() const noexcept { return shared->action; }

const std::optional<Security> Transaction::security() const noexcept { return shared->security; }

std::optional<Security> Transaction::security() noexcept { return shared->security; }

Decimal Transaction::numberOfShares() const noexcept { return shared->numberOfShares; }

Decimal Transaction::sharePrice() const noexcept { return shared->sharePrice; }

Decimal Transaction::commission() const noexcept { return shared->commission; }

Decimal Transaction::totalAmount() const noexcept { return shared->totalAmount; }

bool Transaction::valid() { return shared->valid; }

} // namespace pv
