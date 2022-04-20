#ifndef PV_DATAFILE_H
#define PV_DATAFILE_H

#include <atomic>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/signals2.hpp>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Types.h"

namespace pv {
namespace util {
class NonCopyable {
public:
  NonCopyable() = default;
  NonCopyable(const NonCopyable&) = delete;
};
} // namespace util

class Action;
class Account;
class DataFile;
class Security;

using AccountPtr = std::shared_ptr<Account>;
using SecurityPtr = std::shared_ptr<Security>;

class Security : util::NonCopyable {
private:
  pv::DataFile& dataFile_;
  std::string symbol_;
  std::string name_;
  std::string assetClass_;
  std::string sector_;
  std::map<Date, Decimal> prices_;

  mutable boost::signals2::signal<void(const Date&)> signal_beforePriceChanged;

  mutable boost::signals2::signal<void(const Date&, const std::optional<Decimal>, const std::optional<Decimal>)>
      signal_priceChanged;

public:
  inline static const SecurityPtr None = nullptr;

  Security(pv::DataFile& dataFile, std::string symbol, std::string name, std::string assetClass, std::string sector);

  pv::DataFile& dataFile() const noexcept { return dataFile_; }

  std::string symbol() const noexcept { return symbol_; }

  std::string name() const noexcept { return name_; }

  std::string assetClass() const noexcept { return assetClass_; }

  std::string sector() const noexcept { return sector_; }

  const std::map<Date, Decimal>& prices() const noexcept { return prices_; }

  void setPrice(Date date, Decimal price);

  /// @brief Removes a security price.
  /// @return true if the price was removed succesfully
  bool removePrice(Date date);

  boost::signals2::signal<void(const Date&)>& beforePriceChanged() const noexcept { return signal_beforePriceChanged; }

  boost::signals2::signal<void(const Date&, const std::optional<Decimal>, std::optional<Decimal>)>&
  priceChanged() const noexcept {
    return signal_priceChanged;
  }
};

// This class should only be used by pv::Action implementations
class TransactionBase {
public:
  Date date;
  const SecurityPtr security;
  Decimal numberOfShares;
  Decimal sharePrice;
  Decimal commission;
  Decimal totalAmount;

public:
  TransactionBase(pv::Date date, const SecurityPtr security, Decimal numberOfShares, Decimal sharePrice,
                  Decimal commission, Decimal totalAmount)
      : date(date), security(security), numberOfShares(numberOfShares), sharePrice(sharePrice), commission(commission),
        totalAmount(totalAmount) {}
};

class Action {
public:
  Action() = default;

  virtual void processTransaction(TransactionBase& in) const = 0;

  virtual std::string name() const noexcept = 0;
};

class Transaction : util::NonCopyable {
private:
  Account& account_;
  unsigned int id_;
  Date date_;
  const Action& action_;
  const SecurityPtr security_;
  Decimal numberOfShares_;
  Decimal sharePrice_;
  Decimal commission_;
  Decimal totalAmount_;

public:
  Transaction(Account& account, unsigned int id, const Action& action, TransactionBase base)
      : account_(account), id_(id), date_(base.date), action_(action), security_(base.security),
        numberOfShares_(base.numberOfShares), sharePrice_(base.sharePrice), commission_(base.commission),
        totalAmount_(base.totalAmount) {}

  unsigned int id() const noexcept { return id_; }

  Account& account() const noexcept { return account_; }
  Date date() const noexcept { return date_; }

  const Action& action() const noexcept { return action_; }

  const SecurityPtr security() const noexcept { return security_; }

  Decimal numberOfShares() const noexcept { return numberOfShares_; }

  Decimal sharePrice() const noexcept { return sharePrice_; }

  Decimal commission() const noexcept { return commission_; }

  Decimal totalAmount() const noexcept { return totalAmount_; }
};

using TransactionPtr = std::shared_ptr<Transaction>;

class Account : public util::NonCopyable {
private:
  DataFile& dataFile_;
  unsigned int id_;
  std::string name_;

  std::atomic_uint nextTransactionId;
  std::vector<TransactionPtr> transactions_;

  mutable boost::signals2::signal<void()> signal_beforeTransactionAdded;
  mutable boost::signals2::signal<void(const TransactionPtr)> signal_transactionAdded;
  mutable boost::signals2::signal<void(const std::string&, const std::string&)> signal_nameChanged;

public:
  Account(DataFile& dataFile, unsigned int id, std::string name) : dataFile_(dataFile), id_(id), name_(name) {}

  TransactionPtr transactionForId(unsigned int id) {
    for (TransactionPtr transaction : transactions_) {
      if (transaction->id() == id) {
        return transaction;
      }
    }

    return nullptr;
  }

  DataFile& dataFile() const noexcept { return dataFile_; }

  unsigned int id() const noexcept { return id_; }

  std::string name() const noexcept { return name_; }

  void setName(std::string name) noexcept {
    std::string oldName = name_;
    name_ = name;
    signal_nameChanged(name, name_);
  }

  const std::vector<TransactionPtr>& transactions() const noexcept { return transactions_; }

  boost::signals2::signal<void()>& beforeTransactionAdded() const noexcept { return signal_beforeTransactionAdded; }

  boost::signals2::signal<void(const TransactionPtr)>& transactionAdded() const noexcept {
    return signal_transactionAdded;
  }

  boost::signals2::signal<void(const std::string&, const std::string&)>& nameChanged() const noexcept {
    return signal_nameChanged;
  }

  TransactionPtr addTransaction(pv::Date date, const pv::Action& action, const SecurityPtr security,
                                pv::Decimal numberOfShares, pv::Decimal sharePrice, pv::Decimal commission,
                                pv::Decimal totalAmount);
};
class DataFile : util::NonCopyable {
private:
  std::atomic_uint nextAccountId;

  std::vector<SecurityPtr> securities_;
  std::vector<AccountPtr> accounts_;

  mutable boost::signals2::signal<void(AccountPtr)> signal_accountAdded;
  mutable boost::signals2::signal<void(SecurityPtr)> signal_securityAdded;
  mutable boost::signals2::signal<void()> signal_beforeSecurityAdded;

public:
  DataFile() = default;

  const std::vector<AccountPtr>& accounts() const noexcept { return accounts_; }

  const std::vector<SecurityPtr>& securities() const noexcept { return securities_; }

  boost::signals2::signal<void(AccountPtr)>& accountAdded() const noexcept { return signal_accountAdded; }

  boost::signals2::signal<void(SecurityPtr)>& securityAdded() { return signal_securityAdded; }

  boost::signals2::signal<void()>& beforeSecurityAdded() { return signal_beforeSecurityAdded; }

  AccountPtr accountForId(unsigned int id) {
    for (AccountPtr account : accounts_) {
      if (account->id() == id) {
        return account;
      }
    }

    return nullptr;
  }

  SecurityPtr securityForSymbol(std::string symbol) {
    for (SecurityPtr security : securities_) {
      if (security->symbol() == symbol) {
        return security;
      }
    }

    return nullptr;
  }

  AccountPtr addAccount(std::string name);
  SecurityPtr addSecurity(std::string symbol, std::string name, std::string assetClass, std::string sector);
};
} // namespace pv

#endif // PV_DATAFILE_H
