#ifndef PV_ACCOUNT_H
#define PV_ACCOUNT_H

#include "pv/DataFile.h"
#include "pv/Integer64.h"
#include <string>

namespace pv {
namespace account {

std::string name(const DataFile& dataFile, i64 account);

} // namespace account
} // namespace pv
#endif // PV_ACCOUNT_H
