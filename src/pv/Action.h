#ifndef PV_ACTION_H
#define PV_ACTION_H

#include "Date.h"
#include "Decimal.h"
#include "Security.h"
#include "Transaction.h"
#include <optional>
#include <string>

namespace pv {

class Action {
public:
  Action() = default;

  // Disable copying, moving
  Action(Action&) = delete;
  void operator=(Action&) = delete;
  Action(Action&&) = delete;
  void operator=(Action&&) = delete;

  virtual void processTransactionParamaters(Date& date, std::optional<const Security>& security,
                                            Decimal& numberOfShares, Decimal& sharePrice, Decimal& commission,
                                            Decimal& totalAmount) const noexcept = 0;
  virtual Decimal cashBalance(const Date& /* date */, const std::optional<const Security>& /* security */,
                              const Decimal& /* numberOfShares */, const Decimal& /* sharePrice */,
                              const Decimal& /* commission */, const Decimal& /* totalAmount */) const noexcept {
    return 0;
  }

  Decimal cashBalance(const Transaction& t) const noexcept {
    return cashBalance(t.date(), t.security(), t.numberOfShares(), t.sharePrice(), t.commission(), t.totalAmount());
  }

  virtual std::string id() const noexcept = 0;
};

} // namespace pv

#endif // PV_ACTION_H
