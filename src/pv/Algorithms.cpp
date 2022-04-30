#include "Algorithms.h"
#include "pv/Action.h"
#include "pv/Transaction.h"
#include <algorithm>
#include <execution>
#include <vector>

namespace pv {
namespace algorithms {

Decimal cashBalance(const Account& account) noexcept {
  std::vector<Decimal> transactionCashBalances;
  transactionCashBalances.reserve(account.transactions().size());
  std::transform(account.transactions().cbegin(), account.transactions().cend(),
                 std::back_inserter(transactionCashBalances),
                 [](const pv::Transaction& t) { return t.action().cashBalance(t); });
  return std::reduce(transactionCashBalances.cbegin(), transactionCashBalances.cend());
}

} // namespace algorithms
} // namespace pv
