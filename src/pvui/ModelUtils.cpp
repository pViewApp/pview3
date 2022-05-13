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

} // namespace util
} // namespace models
} // namespace pvui
