#ifndef PV_TRANSACTION_H
#define PV_TRANSACTION_H

#include "pv/DataFile.h"
#include "pv/Integer64.h"

namespace pv {
namespace transaction {

i64 date(const DataFile& dataFile, pv::i64 transaction) noexcept;
i64 account(const DataFile& dataFile, pv::i64 transaction) noexcept; 
pv::Action action(const DataFile& dataFile, pv::i64 transaction) noexcept;

i64 buySecurity(const DataFile& dataFile, pv::i64 transaction) noexcept;
i64 buyNumberOfShares(const DataFile& dataFile, pv::i64 transaction) noexcept;
i64 buySharePrice(const DataFile& dataFile, pv::i64 transaction) noexcept;
i64 buyCommission(const DataFile& dataFile, pv::i64 transaction) noexcept;
i64 buyAmount(const DataFile& dataFile, pv::i64 transaction) noexcept;

i64 sellSecurity(const DataFile& dataFile, pv::i64 transaction) noexcept;
i64 sellNumberOfShares(const DataFile& dataFile, pv::i64 transaction) noexcept;
i64 sellSharePrice(const DataFile& dataFile, pv::i64 transaction) noexcept;
i64 sellCommission(const DataFile& dataFile, pv::i64 transaction) noexcept;
i64 sellAmount(const DataFile& dataFile, pv::i64 transaction) noexcept;

std::optional<i64> depositSecurity(const DataFile& dataFile, pv::i64 transaction) noexcept;
i64 depositAmount(const DataFile& dataFile, pv::i64 transaction) noexcept;

std::optional<i64> withdrawSecurity(const DataFile& dataFile, pv::i64 transaction) noexcept;
i64 withdrawAmount(const DataFile& dataFile, pv::i64 transaction) noexcept;

i64 dividendSecurity(const DataFile& dataFile, pv::i64 transaction) noexcept;
i64 dividendAmount(const DataFile& dataFile, pv::i64 transaction) noexcept;

}
} // namespace pv

#endif // PV_TRANSACTION_H
