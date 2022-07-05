#ifndef PV_SECURITY_H
#define PV_SECURITY_H

#include "DataFile.h"
#include "pv/Integer64.h"
#include <string>

namespace pv {
namespace security {

std::string symbol(const DataFile& dataFile, i64 security) noexcept;
std::string name(const DataFile& dataFile, i64 security) noexcept;
std::string assetClass(const DataFile& dataFile, i64 security) noexcept;
std::string sector(const DataFile& dataFile, i64 security) noexcept;

std::optional<pv::i64> price(const DataFile& dataFile, i64 security, i64 date);

std::optional<pv::i64> securityForSymbol(const DataFile& dataFile, std::string symbol);

}
} // namespace pv

#endif // PV_SECURITY_H
