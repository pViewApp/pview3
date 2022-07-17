#ifndef PV_SECURITY_H
#define PV_SECURITY_H

#include "DataFile.h"
#include "pv/Integer64.h"
#include <string>

namespace pv {
namespace security {

std::string symbol(DataFile& dataFile, i64 security) noexcept;
std::string name(DataFile& dataFile, i64 security) noexcept;
std::string assetClass(DataFile& dataFile, i64 security) noexcept;
std::string sector(DataFile& dataFile, i64 security) noexcept;

std::optional<pv::i64> price(DataFile& dataFile, i64 security, i64 date);

std::optional<pv::i64> securityForSymbol(DataFile& dataFile, std::string symbol);
}
} // namespace pv

#endif // PV_SECURITY_H
