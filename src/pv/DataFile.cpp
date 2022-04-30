#include "DataFile.h"
#include <algorithm>

namespace pv {
DataFile::~DataFile() {
  std::vector<Account> accountsCopy = accounts_;
  std::vector<Security> securitiesCopy = securities_;

  for (auto& account : accountsCopy) {
    removeAccount(account);
  }
  for (auto& security : securitiesCopy) {
    removeSecurity(security);
  }
}

std::optional<Account> DataFile::accountForId(unsigned int id) const noexcept {
  auto iter =
      std::find_if(accounts_.cbegin(), accounts_.cend(), [&](const Account& account) { return account.id() == id; });

  if (iter == accounts_.cend())
    return std::nullopt;
  else
    return Account(*iter);
}

std::optional<Security> DataFile::securityForSymbol(std::string symbol) const noexcept {
  auto iter = std::find_if(securities_.cbegin(), securities_.cend(),
                           [&](const Security& security) { return security.symbol() == symbol; });

  if (iter == securities_.cend())
    return std::nullopt;
  else
    return Security(*iter);
}

Account DataFile::addAccount(std::string name) noexcept {
  Account account(*this, nextAccountId.fetch_add(1), name);
  accounts_.push_back(account);
  signal_accountAdded(account);
  return account;
}

Security DataFile::addSecurity(std::string symbol, std::string name, std::string assetClass,
                               std::string sector) noexcept {
  Security security(*this, symbol, name, assetClass, sector);
  securities_.push_back(security);
  signal_securityAdded(security);
  return security;
}

bool DataFile::removeAccount(Account account) {
  if (account.dataFile() != this || !account.valid())
    return false;
  account.invalidate();
  accounts_.erase(std::find(accounts_.cbegin(), accounts_.cend(), account));
  signal_accountRemoved(account);
  return true;
}

bool DataFile::removeSecurity(Security security) {
  if (security.dataFile() != this || !security.valid())
    return false;
  security.invalidate();
  securities_.erase(std::find(securities_.cbegin(), securities_.cend(), security));
  signal_securityRemoved(security);
  return true;
}

} // namespace pv
