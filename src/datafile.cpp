#include "DataFile.h"
#include <optional>

pv::TransactionPtr pv::Account::addTransaction(pv::Date date, const pv::Action& action, const pv::SecurityPtr security,
                                               pv::Decimal numberOfShares, pv::Decimal sharePrice,
                                               pv::Decimal commission, pv::Decimal totalAmount) {
  pv::TransactionBase base(date, security, numberOfShares, sharePrice, commission, totalAmount);
  action.processTransaction(base);

  signal_beforeTransactionAdded();

  unsigned int id = nextTransactionId.fetch_add(1);
  pv::TransactionPtr transaction = std::make_shared<pv::Transaction>(*this, id, action, base);

  transactions_.push_back(transaction);

  signal_transactionAdded(transaction);

  return transaction;
}

pv::AccountPtr pv::DataFile::addAccount(std::string name) {
  unsigned int id = nextAccountId.fetch_add(1);

  pv::AccountPtr account = std::make_shared<Account>(*this, id, name);
  accounts_.push_back(account);

  signal_accountAdded(account);

  return account;
}

pv::SecurityPtr pv::DataFile::addSecurity(std::string symbol, std::string name, std::string assetClass,
                                          std::string sector) {
  signal_beforeSecurityAdded();

  pv::SecurityPtr security = std::make_shared<pv::Security>(*this, symbol, name, assetClass, sector);
  securities_.push_back(security);

  signal_securityAdded(security);

  return security;
}

pv::Security::Security(pv::DataFile& dataFile, std::string symbol, std::string name, std::string assetClass,
                       std::string sector)
    : dataFile_(dataFile), symbol_(symbol), name_(name), assetClass_(assetClass), sector_(sector) {}

void pv::Security::setPrice(pv::Date date, pv::Decimal price) {
  signal_beforePriceChanged(date);

  auto oldPriceIter = prices_.find(date);
  std::optional<pv::Decimal> oldPrice =
      oldPriceIter == prices_.cend() ? std::optional<pv::Decimal>(std::nullopt) : std::optional(oldPriceIter->second);

  prices_.insert_or_assign(date, price);

  signal_priceChanged(date, oldPrice, price);
}

bool pv::Security::removePrice(Date date) {
  signal_beforePriceChanged(date);

  auto oldPriceIter = prices_.find(date);
  if (oldPriceIter == prices_.cend())
    return false;

  std::optional<pv::Decimal> oldPrice = oldPriceIter->second;

  prices_.erase(oldPriceIter);

  signal_priceChanged(date, oldPrice, std::nullopt);

  return true;
}
