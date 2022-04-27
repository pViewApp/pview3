#ifndef PV_ACTIONS_H
#define PV_ACTIONS_H

#include "Action.h"
#include "pv/Security.h"

namespace {

class BuySellActionBase : public pv::Action {
public:
  void processTransactionParamaters(pv::Date, std::optional<pv::Security>, pv::Decimal numberOfShares,
                                    pv::Decimal sharePrice, pv::Decimal commission,
                                    pv::Decimal totalAmount) const noexcept {
    totalAmount = (numberOfShares * sharePrice) + commission;
  };
};

class BuyAction : public BuySellActionBase {
public:
  std::string id() const noexcept { return "pv/buy"; }
};

class SellAction : public BuySellActionBase {
public:
  std::string id() const noexcept { return "pv/sell"; }
};

inline BuyAction BUY_;
inline SellAction SELL_;

} // namespace

namespace pv::actions {

inline const pv::Action& BUY = BUY_;
inline const pv::Action& SELL = SELL_;

} // namespace pv::actions

#endif // PV_ACTIONS_H
