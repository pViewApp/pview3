#include "DataFile.h"
#include <algorithm>

namespace pv {

void DataFile::removeAccountImpl(const Account& account) noexcept {
  accounts_.erase(std::find(accounts_.cbegin(), accounts_.cend(), account));
  signal_accountRemoved(account);
  accountInvalidationConnections.erase(account);
}

void DataFile::removeSecurityImpl(const Security& security) noexcept {
  securities_.erase(std::find(securities_.cbegin(), securities_.cend(), security));
  signal_securityRemoved(security);
  securityInvalidationConnections.erase(security);
}

DataFile::~DataFile() {
  accountInvalidationConnections.clear();
  securityInvalidationConnections.clear();

  for (auto& account : accounts_) {
    account.invalidate();
  }
  for (auto& security : securities_) {
    security.invalidate();
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

  accountInvalidationConnections.insert(
      {account, account.invalidated().connect([=]() { removeAccountImpl(account); })});

  signal_accountAdded(account);

  return account;
}

Security DataFile::addSecurity(std::string symbol, std::string name, std::string assetClass,
                               std::string sector) noexcept {
  Security security(*this, symbol, name, assetClass, sector);
  securities_.push_back(security);
  securityInvalidationConnections.insert(
      {security, security.invalidated().connect([=]() { removeSecurityImpl(security); })});

  signal_securityAdded(security);

  return security;
}

} // namespace pv
