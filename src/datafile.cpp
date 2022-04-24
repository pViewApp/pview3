#include "DataFile.h"
#include <optional>

pv::TransactionPtr pv::Account::addTransaction(pv::Date date, const pv::Action& action, const pv::SecurityPtr security,
                                               pv::Decimal numberOfShares, pv::Decimal sharePrice,
                                               pv::Decimal commission, pv::Decimal totalAmount) {
  if (!isValid())
    return nullptr;
  pv::TransactionBase base(date, security, numberOfShares, sharePrice, commission, totalAmount);
  action.processTransaction(base);

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
  pv::SecurityPtr security = std::make_shared<pv::Security>(*this, symbol, name, assetClass, sector);
  securities_.push_back(security);

  signal_securityAdded(security);

  return security;
}

bool pv::DataFile::removeAccount(AccountPtr account) noexcept {
  if (&(account->dataFile()) != this)
    return false;
  if (!account->isValid())
    return false;

  accounts_.erase(std::find(accounts_.begin(), accounts_.end(), account));

  account->invalidate();

  signal_accountRemoved(account);

  return true;
}

bool pv::DataFile::removeSecurity(SecurityPtr security) noexcept {
  if (&(security->dataFile()) != this)
    return false;

  if (!security->isValid())
    return false;

  security->invalidate();

  auto iter = std::find(securities_.cbegin(), securities_.cend(), security);
  securities_.erase(iter);

  signal_securityRemoved(security);

  return true;
}

pv::Security::Security(pv::DataFile& dataFile, std::string symbol, std::string name, std::string assetClass,
                       std::string sector)
    : dataFile_(dataFile), symbol_(symbol), name_(name), assetClass_(assetClass), sector_(sector) {}

bool pv::Security::setName(std::string name) {
  std::string oldName = name_;
  name_ = name;
  signal_nameChanged(name_, oldName);

  return true;
}

bool pv::Security::setAssetClass(std::string name) {
  if (!isValid())
    return false;
  std::string oldAssetClass = assetClass_;
  assetClass_ = name;
  signal_assetClassChanged(name_, oldAssetClass);

  return true;
}

bool pv::Security::setSector(std::string name) {
  if (!isValid())
    return false;
  std::string oldSector = sector_;
  sector_ = name;
  signal_sectorChanged(name_, oldSector);

  return true;
}

bool pv::Security::setPrice(pv::Date date, pv::Decimal price) {
  if (!isValid())
    return false;
  auto oldPriceIter = prices_.find(date);
  std::optional<pv::Decimal> oldPrice =
      oldPriceIter == prices_.cend() ? std::optional<pv::Decimal>(std::nullopt) : std::optional(oldPriceIter->second);

  prices_.insert_or_assign(date, price);

  signal_priceChanged(date, oldPrice, price);

  return true;
}

bool pv::Security::removePrice(Date date) {
  if (!isValid())
    return false;
  auto oldPriceIter = prices_.find(date);
  if (oldPriceIter == prices_.cend())
    return false;

  std::optional<pv::Decimal> oldPrice = oldPriceIter->second;

  prices_.erase(oldPriceIter);

  signal_priceChanged(date, oldPrice, std::nullopt);

  return true;
}

bool pv::Account::setName(std::string name) noexcept {
  if (!isValid())
    return false;
  std::string oldName = name_;
  name_ = name;
  signal_nameChanged(name_, oldName);

  return true;
}
