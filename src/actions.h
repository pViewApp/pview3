#ifndef PV_ACTIONS_H
#define PV_ACTIONS_H

#include "DataFile.h"
#include <array>

namespace {
class BuySellAction : public pv::Action {
public:
  inline void processTransaction(pv::TransactionBase &in) const override {
    in.totalAmount = (in.numberOfShares * in.sharePrice) + in.commission;
  }
};

class BuyAction : public BuySellAction {
public:
  inline std::string name() const noexcept override { return "Buy"; }
};

class SellAction : public BuySellAction {
public:
  inline std::string name() const noexcept override { return "Sell"; }
};
} // namespace

namespace pv {
namespace actions {
static BuyAction BUY = BuyAction();
static SellAction SELL = SellAction();
} // namespace actions

inline const std::array<pv::Action *, 2> ACTIONS{&actions::BUY, &actions::SELL};

const pv::Action *actionFromName(std::string name);
} // namespace pv
#endif // PV_ACTIONS_H
