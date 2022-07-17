#ifndef PV_TRANSACTION_H
#define PV_TRANSACTION_H

#include "pv/DataFile.h"
#include "pv/Integer64.h"

namespace pv {
namespace transaction {

i64 date(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 account(DataFile& dataFile, pv::i64 transaction) noexcept;
pv::Action action(DataFile& dataFile, pv::i64 transaction) noexcept;

i64 buySecurity(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 buyNumberOfShares(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 buySharePrice(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 buyCommission(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 buyAmount(DataFile& dataFile, pv::i64 transaction) noexcept;

i64 sellSecurity(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 sellNumberOfShares(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 sellSharePrice(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 sellCommission(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 sellAmount(DataFile& dataFile, pv::i64 transaction) noexcept;

std::optional<i64> depositSecurity(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 depositAmount(DataFile& dataFile, pv::i64 transaction) noexcept;

std::optional<i64> withdrawSecurity(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 withdrawAmount(DataFile& dataFile, pv::i64 transaction) noexcept;

i64 dividendSecurity(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 dividendAmount(DataFile& dataFile, pv::i64 transaction) noexcept;

i64 interestSecurity(DataFile& dataFile, pv::i64 transaction) noexcept;
i64 interestAmount(DataFile& dataFile, pv::i64 transaction) noexcept;
}
} // namespace pv

#endif // PV_TRANSACTION_H
