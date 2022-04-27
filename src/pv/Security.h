#ifndef PV_SECURITY_H
#define PV_SECURITY_H

#include "Date.h"
#include "Decimal.h"
#include <boost/signals2/signal.hpp>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace pv {

class DataFile;

class Security {
private:
  class Shared;

  std::shared_ptr<Shared> shared;

  friend class DataFile;
  friend struct std::hash<Security>;

  Security(DataFile& dataFile, std::string symbol, std::string name, std::string assetClass, std::string sector);

  bool invalidate() noexcept;

public:
  Security(const Security&) = default;
  Security& operator=(const Security&) = default;

  const pv::DataFile* dataFile() const noexcept;
  pv::DataFile* dataFile() noexcept;

  std::string symbol() const noexcept;
  std::string name() const noexcept;
  std::string assetClass() const noexcept;
  std::string sector() const noexcept;

  bool setName(std::string name) noexcept;
  bool setAssetClass(std::string assetClass) noexcept;
  bool setSector(std::string sector) noexcept;

  bool setPrice(Date date, Decimal price) noexcept;

  bool valid() const noexcept;

  const std::map<Date, Decimal>& prices() const noexcept;

  bool removePrice(Date date) noexcept;

  // Signals

  /// \brief Fired whenever the name of this security changes
  /// # Signal Arguments
  /// \arg \c 1 the new name
  /// \arg \c 2 the old name
  boost::signals2::signal<void(std::string, std::string)>& nameChanged() const noexcept;

  /// \brief Fired whenever the asset class of this security changes
  /// # Signal Arguments
  /// \arg \c 1 the new asset class
  /// \arg \c 2 the old asset class
  boost::signals2::signal<void(std::string, std::string)>& assetClassChanged() const noexcept;

  /// \brief Fired whenever the sector of this security changes
  /// # Signal Arguments
  /// \arg \c 1 the new sector
  /// \arg \c 2 the old sector
  boost::signals2::signal<void(std::string, std::string)>& sectorChanged() const noexcept;

  /// \brief Fired whenever a price changes, is added, or is removed.
  /// # Signal Arguments
  /// \arg \c 1 the date of the price
  /// \arg \c 2 the new price, or an empty std::optional if none
  /// \arg \c 3 the old price, or an empty std::optional if none
  boost::signals2::signal<void(Date, std::optional<Decimal>, std::optional<Decimal>)>& priceChanged() const noexcept;

  boost::signals2::signal<void()>& invalidated() const noexcept;

  // Operators

  bool operator==(const Security& other) const noexcept { return shared == other.shared; }
};

} // namespace pv

template <> struct std::hash<pv::Security> {
private:
  inline static const std::hash<std::shared_ptr<pv::Security::Shared>> sharedHasher;

public:
  std::size_t operator()(const pv::Security& security) const noexcept { return sharedHasher(security.shared); }
};

#endif // PV_SECURITY_H
