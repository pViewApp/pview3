#include "FormatUtils.h"

namespace pvui {
namespace util {

QString formatMoney(const pv::Decimal& money) {
  if (money < 0) {
    return QStringLiteral("-$%L1").arg(static_cast<double>(-money), 0, 'f', 2);
  } else {
    return QStringLiteral("$%L1").arg(static_cast<double>(money), 0, 'f', 2);
  }
}

QString formatPercentage(const pv::Decimal& percentage) {
  return QStringLiteral("%1%").arg(static_cast<double>(percentage), 0, 'f', 2);
}

} // namespace util
} // namespace pvui
