#include "FormatUtils.h"

namespace pvui {
namespace util {

QString formatMoney(const pv::Decimal& money) {
  if (money < 0) {
    // Format negative number

    pv::Decimal positive = -money;
    return QString("$-%1").arg(QString::fromStdString(positive.str()));
  } else {
    return QString("$%1").arg(QString::fromStdString(money.str()));
  }
}

QString formatPercentage(const pv::Decimal& percentage) {
  return QString::fromUtf8("%1%").arg(QString::fromStdString(percentage.str()));
}

} // namespace util
} // namespace pvui
