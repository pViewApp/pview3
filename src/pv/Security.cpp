#include "Security.h"
#include <atomic>

namespace pv {

class Security::Shared {
private:
  friend class Security;

  std::atomic_bool valid = true;

  // Pointer might be invalid
  DataFile* dataFile;

  std::string symbol;
  std::string name;
  std::string assetClass;
  std::string sector;

  std::map<Date, Decimal> prices;

  mutable boost::signals2::signal<void(std::string, std::string)> signal_nameChanged;
  mutable boost::signals2::signal<void(std::string, std::string)> signal_assetClassChanged;
  mutable boost::signals2::signal<void(std::string, std::string)> signal_sectorChanged;
  mutable boost::signals2::signal<void(Date, std::optional<Decimal>, std::optional<Decimal>)> signal_priceChanged;

  mutable boost::signals2::signal<void()> signal_invalidated;

  // Implemented here because we need to call in destructor
  bool invalidate() noexcept {
    bool wasValid = valid.exchange(false);
    if (!wasValid) {
      return false;
    }
    signal_invalidated();
    return true;
  }

public:
  // Constructor is public because of std::make_shared
  Shared(DataFile& dataFile, std::string symbol, std::string name, std::string assetClass, std::string sector)
      : dataFile(&dataFile), symbol(symbol), name(name), assetClass(assetClass), sector(sector) {}

  ~Shared() { invalidate(); }
};

Security::Security(DataFile& dataFile, std::string symbol, std::string name, std::string assetClass, std::string sector)
    : shared(std::make_shared<Security::Shared>(dataFile, symbol, name, assetClass, sector)) {}

bool Security::invalidate() noexcept { return shared->invalidate(); }

const DataFile* Security::dataFile() const noexcept { return valid() ? shared->dataFile : nullptr; }

DataFile* Security::dataFile() noexcept { return valid() ? shared->dataFile : nullptr; }

std::string Security::symbol() const noexcept { return shared->symbol; }

std::string Security::name() const noexcept { return shared->name; }

std::string Security::assetClass() const noexcept { return shared->assetClass; }

std::string Security::sector() const noexcept { return shared->sector; }

bool Security::setName(std::string name) noexcept {
  if (!valid())
    return false;
  std::string oldName = std::move(shared->name);
  shared->name = name;
  shared->signal_nameChanged(name, oldName);

  return true;
}

bool Security::setAssetClass(std::string assetClass) noexcept {
  if (!valid())
    return false;
  std::string oldAssetClass = std::move(shared->assetClass);
  shared->assetClass = assetClass;
  shared->signal_assetClassChanged(assetClass, oldAssetClass);

  return true;
}

bool Security::setSector(std::string sector) noexcept {
  if (!valid())
    return false;
  std::string oldSector = std::move(shared->sector);
  shared->sector = sector;
  shared->signal_sectorChanged(sector, oldSector);

  return true;
}

bool Security::setPrice(Date date, Decimal price) noexcept {
  if (!valid()) {
    return false;
  }
  auto oldPriceIter = shared->prices.find(date);
  std::optional<Decimal> oldPrice;

  if (oldPriceIter != shared->prices.cend()) {
    oldPrice = oldPriceIter->second;
  }

  shared->prices.insert_or_assign(date, price);

  shared->signal_priceChanged(date, price, oldPrice);
  return true;
}

bool Security::valid() const noexcept { return shared->valid; }

const std::map<Date, Decimal>& Security::prices() const noexcept { return shared->prices; }

bool Security::removePrice(Date date) noexcept {
  if (!valid()) {
    return false;
  }
  auto oldPriceIter = shared->prices.find(date);
  if (oldPriceIter == shared->prices.cend()) {
    return false;
  }

  auto oldPrice = oldPriceIter->second;

  shared->prices.erase(oldPriceIter);

  shared->signal_priceChanged(date, std::nullopt, oldPrice);

  return true;
}

boost::signals2::signal<void(std::string, std::string)>& Security::nameChanged() const noexcept {
  return shared->signal_nameChanged;
}

boost::signals2::signal<void(std::string, std::string)>& Security::assetClassChanged() const noexcept {
  return shared->signal_sectorChanged;
}

boost::signals2::signal<void(std::string, std::string)>& Security::sectorChanged() const noexcept {
  return shared->signal_sectorChanged;
}

boost::signals2::signal<void(Date, std::optional<Decimal>, std::optional<Decimal>)>&
Security::priceChanged() const noexcept {
  return shared->signal_priceChanged;
}

boost::signals2::signal<void()>& Security::invalidated() const noexcept { return shared->signal_invalidated; }

bool Security::operator<(const Security& other) const noexcept {
  return shared->dataFile < other.shared->dataFile ? true : symbol() < other.symbol();
}

bool Security::operator>(const Security& other) const noexcept {
  return shared->dataFile > other.shared->dataFile ? true : symbol() > other.symbol();
}

} // namespace pv
