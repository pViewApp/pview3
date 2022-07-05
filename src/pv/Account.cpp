#include "Account.h"
#include "DataFile.h"
#include "Integer64.h"
#include <sqlite3.h>

namespace pv {
namespace account {

std::string name(const DataFile& dataFile, i64 account) {
  auto query = dataFile.query("SELECT Name FROM Accounts WHERE Id = ?");
  sqlite3_bind_int64(&*query, 1, static_cast<sqlite3_int64>(account));
  sqlite3_step(&*query);
  const unsigned char* cStr = sqlite3_column_text(&*query, 0);
  return std::string(cStr, cStr + sqlite3_column_bytes(&*query, 0));
}

} // namespace account
} // namespace pv
