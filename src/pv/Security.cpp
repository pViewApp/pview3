#include "Security.h"
#include <atomic>

namespace pv {

Security::Security(DataFile& dataFile, std::string symbol, std::string name, std::string assetClass, std::string sector)
    : dataFile_(&dataFile), symbol_(symbol), name_(name), assetClass_(assetClass), sector_(sector) {}

const DataFile& Security::dataFile() const noexcept { return *dataFile_; }

Connection Security::listenNameChanged(const NameChangedSignal::slot_type& slot) noexcept {
  return signal_nameChanged.connect(slot);
}

Connection Security::listenAssetClassChanged(const AssetClassChangedSignal::slot_type& slot) noexcept {
  return signal_assetClassChanged.connect(slot);
}

Connection Security::listenSectorChanged(const SectorChangedSignal::slot_type& slot) noexcept {
  return signal_sectorChanged.connect(slot);
}

Connection Security::listenPriceChanged(const PriceChangedSignal::slot_type& slot) noexcept {
  return signal_priceChanged.connect(slot);
}

std::string Security::symbol() const noexcept { return symbol_; }

std::string Security::name() const noexcept { return name_; }

std::string Security::assetClass() const noexcept { return assetClass_; }
std::string Security::sector() const noexcept { return sector_; }

const std::map<Date, Decimal>& Security::prices() const noexcept { return prices_; }

void Security::setName(std::string name) {
  std::string oldName = std::move(name_);
  name_ = name;
  signal_nameChanged(std::move(name), std::move(oldName));
}

void Security::setAssetClass(std::string assetClass) {
  std::string oldAssetClass = std::move(assetClass_);
  assetClass_ = assetClass;
  signal_assetClassChanged(std::move(assetClass), std::move(oldAssetClass));
}

void Security::setSector(std::string sector) {
  std::string oldSector = std::move(sector_);
  sector_ = sector;
  signal_sectorChanged(std::move(sector), std::move(oldSector));
}

bool Security::setPrice(Date date, Decimal price) noexcept {
  const auto oldPriceIter = prices_.find(date);
  std::optional<Decimal> oldPrice = std::nullopt;

  if (oldPriceIter != prices_.cend()) {
    oldPrice = oldPriceIter->second;
  }

  prices_.insert_or_assign(date, price);

  signal_priceChanged(std::move(date), std::move(price), std::move(oldPrice));

  return true;
}

bool Security::removePrice(Date date) noexcept {
  const auto oldPriceIter = prices_.find(date);
  std::optional<Decimal> oldPrice = std::nullopt;

  if (oldPriceIter == prices_.cend()) {
    return false;
  }

  oldPrice = oldPriceIter->second;
  prices_.erase(oldPriceIter);

  signal_priceChanged(std::move(date), std::nullopt, std::move(oldPrice));

  return true;
}

} // namespace pv
