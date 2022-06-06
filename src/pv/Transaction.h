#ifndef PV_TRANSACTION_H
#define PV_TRANSACTION_H

#include "Date.h"
#include "Decimal.h"
#include "Invalidatable.h"
#include "Security.h"

namespace pv {

class Account;
class DataFile;

enum class Action : unsigned char { BUY = 0, SELL = 1, DEPOSIT = 2, WITHDRAW = 3, DIVIDEND = 4 };

struct Transaction {
public:
  pv::Date date;
  virtual Action action() const noexcept = 0;

  virtual ~Transaction() = default;

  Transaction(pv::Date date) noexcept : date(date) {}

  Transaction(Transaction&&) = default;
  Transaction(const Transaction&) = default;
  Transaction& operator=(const Transaction&) = default;
  Transaction& operator=(Transaction&&) = default;
};

struct BuyTransaction : public Transaction {
public:
  pv::Security* security;
  pv::Decimal numberOfShares;
  pv::Decimal sharePrice;
  pv::Decimal commission;

  BuyTransaction(BuyTransaction&&) = default;
  BuyTransaction(const BuyTransaction&) = default;
  BuyTransaction& operator=(const BuyTransaction&) = default;
  BuyTransaction& operator=(BuyTransaction&&) = default;

  Action action() const noexcept override { return Action::BUY; };

  BuyTransaction(pv::Date date, pv::Security* security, pv::Decimal numberOfShares, pv::Decimal sharePrice,
                 pv::Decimal commission)
      : Transaction(date), security(security), numberOfShares(numberOfShares), sharePrice(sharePrice),
        commission(commission) {}
};

struct SellTransaction : public Transaction {
public:
  pv::Security* security;
  pv::Decimal numberOfShares;
  pv::Decimal sharePrice;
  pv::Decimal commission;

  Action action() const noexcept override { return Action::SELL; }

  SellTransaction(SellTransaction&&) = default;
  SellTransaction(const SellTransaction&) = default;
  SellTransaction& operator=(const SellTransaction&) = default;
  SellTransaction& operator=(SellTransaction&&) = default;

  SellTransaction(pv::Date date, pv::Security* security, pv::Decimal numberOfShares, pv::Decimal sharePrice,
                  pv::Decimal commission)
      : Transaction(date), security(security), numberOfShares(numberOfShares), sharePrice(sharePrice),
        commission(commission) {}
};

struct DepositTransaction : public Transaction {
public:
  pv::Security* security;
  pv::Decimal amount;

  DepositTransaction(DepositTransaction&&) = default;
  DepositTransaction(const DepositTransaction&) = default;
  DepositTransaction& operator=(const DepositTransaction&) = default;
  DepositTransaction& operator=(DepositTransaction&&) = default;

  DepositTransaction(pv::Date date, pv::Security* security, pv::Decimal amount) noexcept
      : Transaction(date), security(security), amount(amount) {}

  Action action() const noexcept override { return Action::DEPOSIT; }
};

struct WithdrawTransaction : public Transaction {
public:
  pv::Security* security;
  pv::Decimal amount;

  WithdrawTransaction(WithdrawTransaction&&) = default;
  WithdrawTransaction(const WithdrawTransaction&) = default;
  WithdrawTransaction& operator=(const WithdrawTransaction&) = default;
  WithdrawTransaction& operator=(WithdrawTransaction&&) = default;

  WithdrawTransaction(pv::Date date, pv::Security* security, pv::Decimal amount) noexcept
      : Transaction(date), security(security), amount(amount) {}

  Action action() const noexcept override { return Action::WITHDRAW; }
};

struct DividendTransaction : public Transaction {
public:
  pv::Security* security;
  pv::Decimal amount;

  DividendTransaction(DividendTransaction&&) = default;
  DividendTransaction(const DividendTransaction&) = default;
  DividendTransaction& operator=(const DividendTransaction&) = default;
  DividendTransaction& operator=(DividendTransaction&&) = default;

  DividendTransaction(pv::Date date, pv::Security* security, pv::Decimal amount) noexcept
      : Transaction(date), security(security), amount(amount) {}

  Action action() const noexcept override { return Action::DIVIDEND; }
};

} // namespace pv

#endif // PV_TRANSACTION_H
