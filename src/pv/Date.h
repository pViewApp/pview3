#ifndef PV_DATE_H
#define PV_DATE_H

#include "Integer64.h"
#include <ctime>
#include <cmath>

namespace pv {

namespace dates {

inline i64 today() {
  return static_cast<i64>(std::floor(std::time(nullptr) / 86400.0));
}

} // namespace dates

} // namespace pv

#endif // PV_DATE_H
