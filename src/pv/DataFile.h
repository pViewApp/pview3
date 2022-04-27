#ifndef PV_DATAFILE_H
#define PV_DATAFILE_H

#include "Account.h"
#include "Security.h"
#include <atomic>
#include <boost/signals2.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace pv {

class DataFile {
private:
  std::atomic_uint nextAccountId = 0;

  std::vector<Account> accounts_;
  std::vector<Security> securities_;

  mutable boost::signals2::signal<void(Account&)> signal_accountAdded;
  mutable boost::signals2::signal<void(const Account&)> signal_accountRemoved;

  mutable boost::signals2::signal<void(Security&)> signal_securityAdded;
  mutable boost::signals2::signal<void(const Security&)> signal_securityRemoved;

  std::unordered_map<Account, boost::signals2::scoped_connection> accountInvalidationConnections;
  std::unordered_map<Security, boost::signals2::scoped_connection> securityInvalidationConnections;

  void removeAccountImpl(const Account& account) noexcept;
  void removeSecurityImpl(const Security& account) noexcept;

public:
  DataFile() = default;
  ~DataFile();

  // Disable copy/move
  DataFile(DataFile&) = delete;
  void operator=(DataFile&) = delete;
  DataFile(DataFile&&) = delete;
  void operator=(DataFile&&) = delete;

  const std::vector<Account>& accounts() const noexcept { return accounts_; }

  const std::vector<Security>& securities() const noexcept { return securities_; }

  std::optional<Account> accountForId(unsigned int id) const noexcept;
  std::optional<Security> securityForSymbol(std::string symbol) const noexcept;

  // Mutators

  Account addAccount(std::string name) noexcept;
  Security addSecurity(std::string symbol, std::string name, std::string assetClass, std::string sector) noexcept;

  bool removeAccount(Account account) {
    account.invalidate();
    return true;
  }

  bool removeSecurity(Security security) {
    security.invalidate();
    return true;
  }

  // Signals

  boost::signals2::signal<void(Account&)>& accountAdded() const noexcept { return signal_accountAdded; }

  boost::signals2::signal<void(const Account&)>& accountRemoved() const noexcept { return signal_accountRemoved; }

  boost::signals2::signal<void(Security&)>& securityAdded() const noexcept { return signal_securityAdded; }

  boost::signals2::signal<void(const Security&)>& securityRemoved() const noexcept { return signal_securityRemoved; }
};

} // namespace pv

#endif // PV_DATAFILE_H
