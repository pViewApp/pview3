#ifndef PV_ALGORITHMS_ALGORITHMS_H
#define PV_ALGORITHMS_ALGORITHMS_H

#include "Decimal.h"
#include "pv/DataFile.h"

namespace pv {
namespace algorithms {

Decimal cashBalance(const Account& account) noexcept;

} // namespace algorithms
} // namespace pv

#endif // PV_ALGORITHMS_ALGORITHMS_H
