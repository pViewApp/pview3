#ifndef PV_DATAFILE_H
#define PV_DATAFILE_H

#include "Account.h"
#include "Security.h"
#include "Signals.h"
#include <list>
#include <string>

namespace pv {

enum class AccountRemovalResult : unsigned char {
  SUCCESS = 0,
  ERR_OTHER = 1,
  ERR_INVALID_ACCOUNT = 2,
};

enum class SecurityRemovalResult : unsigned char {
  SUCCESS = 0,
  ERR_OTHER = 1,
  ERR_INVALID_SECURITY = 2,
  ERR_IN_USE = 3,
};

class DataFile {
public:
  using AccountAddedSignal = Signal<void(Account*)>;
  using AccountRemovedSignal = Signal<void(const Account*)>;

  using SecurityAddedSignal = Signal<void(Security*)>;
  using SecurityRemovedSignal = Signal<void(const Security*)>;

private:
  std::vector<Account*> accounts_;
  std::vector<Security*> securities_;

  std::vector<const Account*> accountsConst;
  std::vector<const Security*> securitiesConst;

  AccountAddedSignal signal_accountAdded;

  AccountRemovedSignal signal_accountRemoved;

  SecurityAddedSignal signal_securityAdded;

  SecurityRemovedSignal signal_securityRemoved;

public:
  DataFile() = default;
  DataFile(DataFile&&) = default;
  DataFile& operator=(DataFile&&) = default;

  ~DataFile();
  // Not copyable

  const std::vector<Account*>& accounts() noexcept;
  const std::vector<Security*>& securities() noexcept;

  const std::vector<const Account*>& accounts() const noexcept;
  const std::vector<const Security*>& securities() const noexcept;

  Account& mutableReference(const Account& account);
  Security& mutableReference(const Security& account) noexcept;

  Account* addAccount(std::string name);
  Security* addSecurity(std::string symbol, std::string name, std::string assetClass, std::string sector);

  const pv::Security* securityForSymbol(std::string symbol) const noexcept;
  pv::Security* securityForSymbol(std::string symbol) noexcept;

  AccountRemovalResult removeAccount(Account& account) noexcept;
  SecurityRemovalResult removeSecurity(Security& security) noexcept;

  // listen methods
  boost::signals2::connection listenAccountAdded(const AccountAddedSignal::slot_type& slot);

  boost::signals2::connection listenAccountRemoved(const AccountRemovedSignal::slot_type& slot);

  boost::signals2::connection listenSecurityAdded(const SecurityAddedSignal::slot_type& slot);

  boost::signals2::connection listenSecurityRemoved(const SecurityRemovedSignal::slot_type& slot);

  bool owns(const Account& account) const noexcept;
  bool owns(const Security& security) const noexcept;
};

} // namespace pv

#endif // PV_DATAFILE_H
