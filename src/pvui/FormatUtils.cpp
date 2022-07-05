#include "FormatUtils.h"
#include "pv/Integer64.h"
#include <QtGlobal>

namespace pvui {
namespace util {

QString formatMoney(pv::i64 money) {
  if (money < 0) {
    return QStringLiteral("-$%L1.%2").arg(static_cast<qulonglong>(-money / 100.)).arg(-money % 100, 2, 10, QLatin1Char('0'));
  } else {
    return QStringLiteral("$%L1.%2").arg(static_cast<qulonglong>(money / 100.)).arg(money % 100, 2, 10, QLatin1Char('0'));
  }
}

QString formatPercentage(double percentage) {
  return QStringLiteral("%1%").arg(percentage, 0, 'f', 2);
}

} // namespace util
} // namespace pvui
