#include "DataFile.h"
#include "Algorithms.h"
#include <algorithm>

namespace pv {

DataFile::~DataFile() {
  for (const auto* account : accounts_) {
    delete account;
  }

  for (const auto* security : securities_) {
    delete security;
  }
}

const std::vector<Account*>& DataFile::accounts() noexcept { return accounts_; }

const std::vector<Security*>& DataFile::securities() noexcept { return securities_; }

const std::vector<const Account*>& DataFile::accounts() const noexcept { return accountsConst; }

const std::vector<const Security*>& DataFile::securities() const noexcept { return securitiesConst; }

Account* DataFile::addAccount(std::string name) {
  auto* account = new Account(*this, name);

  try {
    accounts_.push_back(account);
  } catch (...) {
    delete account;
    throw;
  }

  try {
    accountsConst.push_back(account);
  } catch (...) {
    accounts_.erase(accounts_.end() - 1); // Remove the last added account
    delete account;
    throw;
  }

  signal_accountAdded(account);
  return account;
}

Security* DataFile::addSecurity(std::string symbol, std::string name, std::string assetClass, std::string sector) {
  if (securityForSymbol(symbol) != nullptr) {
    return nullptr; // Security already exists
  }

  Security* security = new Security(*this, symbol, name, assetClass, sector);
  try {
    securities_.push_back(security);
  } catch (...) {
    delete security;
    throw;
  }

  try {
    securitiesConst.push_back(security);
  } catch (...) {
    securities_.erase(securities_.end() - 1);
    delete security;
    throw;
  }

  signal_securityAdded(security);
  return security;
}

const Security* DataFile::securityForSymbol(std::string symbol) const noexcept {
  auto iter = std::find_if(securitiesConst.cbegin(), securitiesConst.cend(),
                           [&symbol](const pv::Security* s) { return s->symbol_ == symbol; });
  if (iter == securitiesConst.cend()) {
    return nullptr;
  } else {
    return *iter;
  }
}

Security* DataFile::securityForSymbol(std::string symbol) noexcept {
  auto iter = std::find_if(securities_.cbegin(), securities_.cend(),
                           [&symbol](const pv::Security* s) { return s->symbol_ == symbol; });
  if (iter == securities_.cend()) {
    return nullptr;
  } else {
    return *iter;
  }
}

AccountRemovalResult DataFile::removeAccount(Account& account) noexcept {
  if (!owns(account)) {
    return AccountRemovalResult::ERR_INVALID_ACCOUNT;
  }

  auto iter = std::find(accounts_.begin(), accounts_.end(), &account);
  std::size_t index = iter - accounts_.begin();
  pv::Account* accountPtr = *iter;

  accounts_.erase(iter);
  assert(accountsConst[index] == &account && "Account list indexes do not match");
  accountsConst.erase(accountsConst.begin() + index);

  signal_accountRemoved(accountPtr);

  delete accountPtr; // delete the account

  return AccountRemovalResult::SUCCESS;
}

SecurityRemovalResult DataFile::removeSecurity(Security& security) noexcept {
  if (!owns(security)) {
    return SecurityRemovalResult::ERR_INVALID_SECURITY;
  }

  // Check if this security is still referenced
  // todo improve efficiency of this implementation
  for (const auto& account : accounts_) {
    for (const auto* transaction : account->transactions()) {
      if (algorithms::security(transaction) == &security) {
        return SecurityRemovalResult::ERR_IN_USE;
      }
    }
  }

  auto iter = std::find(securities_.begin(), securities_.end(), &security);
  std::size_t index = iter - securities_.begin();
  pv::Security* securityPtr = *iter;

  securities_.erase(iter);
  assert(securitiesConst[index] == &security && "security list indexes do not match");
  securitiesConst.erase(securitiesConst.begin() + index);

  signal_securityRemoved(securityPtr);

  delete securityPtr; // delete the security

  return SecurityRemovalResult::SUCCESS;
}

bool DataFile::owns(const Account& account) const noexcept {
  return std::addressof(account.dataFile()) == std::addressof(*this);
}

bool DataFile::owns(const Security& security) const noexcept {
  return std::addressof(security.dataFile()) == std::addressof(*this);
}

boost::signals2::connection DataFile::listenAccountAdded(const AccountAddedSignal::slot_type& slot) {
  return signal_accountAdded.connect(slot);
}

boost::signals2::connection DataFile::listenAccountRemoved(const AccountRemovedSignal::slot_type& slot) {
  return signal_accountRemoved.connect(slot);
}

boost::signals2::connection DataFile::listenSecurityAdded(const SecurityAddedSignal::slot_type& slot) {
  return signal_securityAdded.connect(slot);
}

boost::signals2::connection DataFile::listenSecurityRemoved(const SecurityRemovedSignal::slot_type& slot) {
  return signal_securityRemoved.connect(slot);
}

} // namespace pv
