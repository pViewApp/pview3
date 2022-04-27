#ifndef PV_ACTION_H
#define PV_ACTION_H

#include "Date.h"
#include "Decimal.h"
#include <optional>
#include <string>

namespace pv {

class Security;

class Action {
public:
  Action() = default;

  // Disable copying, moving
  Action(Action&) = delete;
  void operator=(Action&) = delete;
  Action(Action&&) = delete;
  void operator=(Action&&) = delete;

  virtual void processTransactionParamaters(Date date, std::optional<Security> security, Decimal numberOfShares,
                                            Decimal sharePrice, Decimal commission,
                                            Decimal totalAmount) const noexcept = 0;
  virtual std::string id() const noexcept = 0;
};

} // namespace pv

#endif // PV_ACTION_H
