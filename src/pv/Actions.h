#ifndef PV_ACTIONS_H
#define PV_ACTIONS_H

#include "Action.h"
#include "pv/Security.h"

namespace {

/// \brief This actions buys shares of a stock.
class BuyAction : public pv::Action {
public:
  std::string id() const noexcept override { return "pv/buy"; }

  pv::Decimal cashBalance(const pv::Date&, const std::optional<const pv::Security>&, const pv::Decimal&,
                          const pv::Decimal&, const pv::Decimal&,
                          const pv::Decimal& totalAmount) const noexcept override {
    return -totalAmount;
  }

  void processTransactionParamaters(pv::Date&, std::optional<const pv::Security>&, pv::Decimal& numberOfShares,
                                    pv::Decimal& sharePrice, pv::Decimal& commission,
                                    pv::Decimal& totalAmount) const noexcept override {
    totalAmount = (numberOfShares * sharePrice) + commission;
  }
};

/// \brief This action sells shares of a stock.
class SellAction : public pv::Action {
public:
  std::string id() const noexcept override { return "pv/sell"; }

  pv::Decimal cashBalance(const pv::Date&, const std::optional<const pv::Security>&, const pv::Decimal&,
                          const pv::Decimal&, const pv::Decimal&,
                          const pv::Decimal& totalAmount) const noexcept override {
    return totalAmount;
  }

  void processTransactionParamaters(pv::Date&, std::optional<const pv::Security>&, pv::Decimal& numberOfShares,
                                    pv::Decimal& sharePrice, pv::Decimal& commission,
                                    pv::Decimal& totalAmount) const noexcept override {
    totalAmount = (numberOfShares * sharePrice) - commission;
  }
};

/// \brief This action deposits money into the account.
class InAction : public pv::Action {
public:
  std::string id() const noexcept override { return "pv/in"; }

  pv::Decimal cashBalance(const pv::Date&, const std::optional<const pv::Security>&, const pv::Decimal&,
                          const pv::Decimal&, const pv::Decimal&,
                          const pv::Decimal& totalAmount) const noexcept override {
    return totalAmount;
  }

  void processTransactionParamaters(pv::Date&, std::optional<const pv::Security>& security, pv::Decimal& numberOfShares,
                                    pv::Decimal& sharePrice, pv::Decimal& commission,
                                    pv::Decimal& totalAmount) const noexcept override {
    security = std::nullopt;
    numberOfShares = 0;
    sharePrice = 0;
    totalAmount = totalAmount - commission;
  }
};

/// \brief This action withdraws money from the account.
class OutAction : public pv::Action {
public:
  std::string id() const noexcept override { return "pv/out"; }

  pv::Decimal cashBalance(const pv::Date&, const std::optional<const pv::Security>&, const pv::Decimal&,
                          const pv::Decimal&, const pv::Decimal&,
                          const pv::Decimal& totalAmount) const noexcept override {
    return -totalAmount;
  }

  void processTransactionParamaters(pv::Date&, std::optional<const pv::Security>& security, pv::Decimal& numberOfShares,
                                    pv::Decimal& sharePrice, pv::Decimal& commission,
                                    pv::Decimal& totalAmount) const noexcept override {
    security = std::nullopt;
    numberOfShares = 0;
    sharePrice = 0;
    totalAmount = totalAmount + commission;
  }
};

inline BuyAction BUY_;
inline SellAction SELL_;
inline InAction IN_;
inline OutAction OUT_;

} // namespace

namespace pv::actions {

inline const pv::Action& BUY = BUY_;
inline const pv::Action& SELL = SELL_;
inline const pv::Action& IN = IN_;
inline const pv::Action& OUT = OUT_;

} // namespace pv::actions

#endif // PV_ACTIONS_H
