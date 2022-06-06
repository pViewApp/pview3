#ifndef PV_SECURITY_H
#define PV_SECURITY_H

#include "Date.h"
#include "Decimal.h"
#include "Invalidatable.h"
#include "Signals.h"
#include <boost/signals2/dummy_mutex.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/signal_type.hpp>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace pv {

class DataFile;

struct Security {
public:
  using NameChangedSignal = Signal<void(std::string, std::string)>;
  using AssetClassChangedSignal = Signal<void(std::string, std::string)>;
  using SectorChangedSignal = Signal<void(std::string, std::string)>;
  using PriceChangedSignal = Signal<void(pv::Date, std::optional<pv::Decimal>, std::optional<pv::Decimal>)>;

private:
  friend class DataFile;
  Security(DataFile&, std::string symbol, std::string name, std::string assetClass, std::string sector);

  const pv::DataFile* dataFile_;
  std::string symbol_;
  std::string name_;
  std::string assetClass_;
  std::string sector_;

  NameChangedSignal signal_nameChanged;
  AssetClassChangedSignal signal_assetClassChanged;
  SectorChangedSignal signal_sectorChanged;
  PriceChangedSignal signal_priceChanged;

  std::map<Date, Decimal> prices_;

public:
  // Getters
  const pv::DataFile& dataFile() const noexcept;
  std::string symbol() const noexcept;
  std::string name() const noexcept;
  std::string assetClass() const noexcept;
  std::string sector() const noexcept;
  const std::map<Date, Decimal>& prices() const noexcept;

  // Mutators
  void setName(std::string name);
  void setAssetClass(std::string assetClass);
  void setSector(std::string sector);

  bool setPrice(pv::Date date, pv::Decimal price) noexcept;
  bool removePrice(pv::Date date) noexcept;

  // Disable move, copy, and default construction
  Security() = delete;
  Security(Security&& other) = delete;
  Security& operator=(Security&& other) = delete;
  Security& operator=(const Security& other) = delete;
  Security(const Security& other) = delete;

  // listen functions
  Connection listenNameChanged(const NameChangedSignal::slot_type& slot) noexcept;
  Connection listenAssetClassChanged(const AssetClassChangedSignal::slot_type& slot) noexcept;
  Connection listenSectorChanged(const SectorChangedSignal::slot_type& slot) noexcept;
  Connection listenPriceChanged(const PriceChangedSignal::slot_type& slot) noexcept;
};

} // namespace pv

#endif // PV_SECURITY_H
