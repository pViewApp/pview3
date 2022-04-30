#include "Transaction.h"
#include "Account.h"
#include "Action.h"
#include <atomic>

namespace pv {

class Transaction::Shared {
private:
  friend class Transaction;

  Account* account_;
  unsigned int id_;
  Date date_;
  const Action& action_;
  std::optional<const Security> security_;
  Decimal numberOfShares_;
  Decimal sharePrice_;
  Decimal commission_;
  Decimal totalAmount_;

  std::atomic_bool valid = true;

  // signals

  mutable boost::signals2::signal<void(const Decimal&, const Decimal&)> signal_numberOfSharesChanged;
  mutable boost::signals2::signal<void(const Decimal&, const Decimal&)> signal_sharePriceChanged;
  mutable boost::signals2::signal<void(const Decimal&, const Decimal&)> signal_commissionChanged;
  mutable boost::signals2::signal<void(const Decimal&, const Decimal&)> signal_totalAmountChanged;
  mutable boost::signals2::signal<void()> signal_changed;
  mutable boost::signals2::signal<void()> signal_invalidated;
  ;

  TransactionEditResult edit(Date date, std::optional<const Security> security, Decimal numberOfShares,
                             Decimal sharePrice, Decimal commission, Decimal totalAmount) {
    if (!valid)
      return TransactionEditResult::InvalidTransaction;
    action_.processTransactionParamaters(date, security, numberOfShares, sharePrice, commission, totalAmount);

    if (numberOfShares != numberOfShares_) {
      const Decimal old = numberOfShares_;
      numberOfShares_ = numberOfShares;
      signal_numberOfSharesChanged(old, numberOfShares);
    }
    if (sharePrice != sharePrice_) {
      const Decimal old = sharePrice_;
      sharePrice_ = sharePrice;
      signal_sharePriceChanged(old, sharePrice);
    }
    if (commission != commission_) {
      const Decimal old = commission_;
      commission_ = commission;
      signal_commissionChanged(old, commission);
    }
    if (totalAmount != totalAmount_) {
      const Decimal old = totalAmount_;
      totalAmount_ = totalAmount;
      signal_totalAmountChanged(old, totalAmount);
    }

    signal_changed();

    return TransactionEditResult::Success;
  }

public:
  Shared(Account& account, unsigned int id, Date date, const Action& action, std::optional<const Security> security,
         Decimal numberOfShares, Decimal sharePrice, Decimal commission, Decimal totalAmount)
      : account_(&account), id_(id), date_(date), action_(action), security_(security), numberOfShares_(numberOfShares),
        sharePrice_(sharePrice), commission_(commission), totalAmount_(totalAmount) {}

  ~Shared() { invalidate(); }

  // Implemented here because we need to call in destructor
  bool invalidate() noexcept {
    bool wasValid = valid.exchange(false);
    if (!wasValid) {
      return false;
    }

    signal_invalidated();
    return wasValid;
  }
};

Transaction::Transaction(Account& account, unsigned int id, Date date, const Action& action,
                         std::optional<const Security> security, Decimal numberOfShares, Decimal sharePrice,
                         Decimal commission, Decimal totalAmount)
    : shared(std::make_shared<Transaction::Shared>(account, id, date, action, security, numberOfShares, sharePrice,
                                                   commission, totalAmount)) {}

bool Transaction::invalidate() noexcept { return shared->invalidate(); }

const Account* Transaction::account() const noexcept { return shared->account_; }

Account* Transaction::account() noexcept { return shared->account_; }

unsigned int Transaction::id() const noexcept { return shared->id_; }

Date Transaction::date() const noexcept { return shared->date_; }

const Action& Transaction::action() const noexcept { return shared->action_; }

const std::optional<Security> Transaction::security() const noexcept { return shared->security_; }

Decimal Transaction::numberOfShares() const noexcept { return shared->numberOfShares_; }

Decimal Transaction::sharePrice() const noexcept { return shared->sharePrice_; }

Decimal Transaction::commission() const noexcept { return shared->commission_; }

Decimal Transaction::totalAmount() const noexcept { return shared->totalAmount_; }

bool Transaction::valid() const noexcept { return shared->valid; }

TransactionEditResult Transaction::setNumberOfShares(Decimal numberOfShares) noexcept {
  return shared->edit(date(), security(), numberOfShares, sharePrice(), commission(), totalAmount());
}

TransactionEditResult Transaction::setSharePrice(Decimal sharePrice) noexcept {
  return shared->edit(date(), security(), numberOfShares(), sharePrice, commission(), totalAmount());
}

TransactionEditResult Transaction::setCommission(Decimal commission) noexcept {
  return shared->edit(date(), security(), numberOfShares(), sharePrice(), commission, totalAmount());
}

TransactionEditResult Transaction::setTotalAmount(Decimal totalAmount) noexcept {
  return shared->edit(date(), security(), numberOfShares(), sharePrice(), commission(), totalAmount);
}

boost::signals2::signal<void(const Decimal&, const Decimal&)>& Transaction::numberOfSharesChanged() const noexcept {
  return shared->signal_numberOfSharesChanged;
}

boost::signals2::signal<void(const Decimal&, const Decimal&)>& Transaction::sharePriceChanged() const noexcept {
  return shared->signal_sharePriceChanged;
}

boost::signals2::signal<void(const Decimal&, const Decimal&)>& Transaction::commissionChanged() const noexcept {
  return shared->signal_commissionChanged;
}

boost::signals2::signal<void(const Decimal&, const Decimal&)>& Transaction::totalAmountChanged() const noexcept {
  return shared->signal_totalAmountChanged;
}

boost::signals2::signal<void()>& Transaction::changed() const noexcept { return shared->signal_changed; }

} // namespace pv
