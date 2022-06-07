#include "FormatUtils.h"

namespace pvui {
namespace util {

QString formatMoney(const pv::Decimal& money, const QLocale& locale) {
  return locale.toCurrencyString(static_cast<double>(money), "$"); // Always use dollars
}

QString formatPercentage(const pv::Decimal& percentage) {
  return QString::fromUtf8("%1%").arg(QString::fromStdString(percentage.str()));
}

} // namespace util
} // namespace pvui
