#include "ModelUtils.h"
#include "FormatUtils.h"
#include <string>

namespace pvui {
namespace models {
namespace util {

QVariant moneyData(pv::Decimal money, int role) {
  if (role == Qt::EditRole) {
    return static_cast<double>(money);
  } else {
    return ::pvui::util::formatMoney(money);
  }
}

QVariant percentageData(pv::Decimal percentage, int role) {
  if (role == Qt::EditRole) {
    return static_cast<double>(percentage);
  } else {
    return ::pvui::util::formatPercentage(percentage);
  }
}

QVariant numberData(pv::Decimal number, int role) {
  if (role == Qt::EditRole) {
    return static_cast<double>(number);
  } else {
    return QString::fromStdString(number.str());
  }
}

} // namespace util
} // namespace models
} // namespace pvui
