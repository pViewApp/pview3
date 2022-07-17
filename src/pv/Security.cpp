#include "Security.h"
#include "pv/DataFile.h"
#include <atomic>
#include <sqlite3.h>

namespace pv {
namespace security {

namespace {

const char* query = "SELECT Symbol, Name, AssetClass, Sector FROM Securities WHERE Id = ?";

// Index of the columns within the above query
constexpr int symbolIndex = 0;
constexpr int nameIndex = 1;
constexpr int assetClassIndex = 2;
constexpr int sectorIndex = 3;

template <int Index> std::string getSecurityField(DataFile& dataFile, i64 security) {
  auto* stmt = dataFile.cachedQuery(query);
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_step(stmt);
  auto* cStr = sqlite3_column_text(stmt, Index);
  auto value = std::string(cStr, cStr + sqlite3_column_bytes(stmt, Index));
  return value;
}
}

std::string symbol(DataFile& dataFile, i64 security) noexcept {
  return getSecurityField<symbolIndex>(dataFile, security);
}

std::string name(DataFile& dataFile, i64 security) noexcept { return getSecurityField<nameIndex>(dataFile, security); }

std::string assetClass(DataFile& dataFile, i64 security) noexcept {
  return getSecurityField<assetClassIndex>(dataFile, security);
}

std::string sector(DataFile& dataFile, i64 security) noexcept {
  return getSecurityField<sectorIndex>(dataFile, security);
}

std::optional<pv::i64> price(DataFile& dataFile, i64 security, i64 date) {
  auto* stmt = dataFile.cachedQuery("SELECT Price FROM SecurityPrices WHERE SecurityId = ? AND Date = ?");
  sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(security));
  sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(date));
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    return sqlite3_column_int64(stmt, 0);
  } else {
    return std::nullopt;
  }
}

std::optional<pv::i64> securityForSymbol(DataFile& dataFile, std::string symbol) {
  auto* stmt = dataFile.cachedQuery("SELECT Id FROM Securities WHERE Symbol = ?");

  sqlite3_bind_text(stmt, 1, symbol.c_str(), static_cast<int>(symbol.length()), SQLITE_STATIC);
  if (sqlite3_step(stmt) != SQLITE_ROW) {
    return std::nullopt;
  } else {
    return sqlite3_column_int64(stmt, 0);
  }
}

} // namespace security
} // namespace pv
