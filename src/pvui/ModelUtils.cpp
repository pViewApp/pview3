#include "ModelUtils.h"
#include "FormatUtils.h"
#include <QColor>
#include <string>

namespace pvui {
namespace models {
namespace util {

namespace {

QVariant formatColor(const pv::Decimal& amount, const FormatFlags& flags) {
  if (flags & FormatFlag::COLOR_NEGATIVE && amount < 0) {
    // Color negative number
    return QColor(Qt::GlobalColor::red);
  }
  if (flags & FormatFlag::COLOR_POSITIVE && amount > 0) {
    return QColor(Qt::GlobalColor::darkGreen);
  } else {
    return QVariant(); // Don't color text
  }
}

} // namespace

QVariant moneyData(pv::Decimal money, int role, FormatFlags flags) {
  if (role == Qt::EditRole) {
    return static_cast<double>(money);
  } else if (role == Qt::ForegroundRole) {
    return formatColor(money, flags);
  } else {
    return ::pvui::util::formatMoney(money);
  }
}

QVariant percentageData(pv::Decimal percentage, int role, FormatFlags flags) {
  if (role == Qt::EditRole) {
    return static_cast<double>(percentage);
  } else if (role == Qt::ForegroundRole) {
    return formatColor(percentage, flags);
  } else {
    return ::pvui::util::formatPercentage(percentage);
  }
}

QVariant numberData(pv::Decimal number, int role, FormatFlags flags) {
  if (role == Qt::EditRole) {
    return static_cast<double>(number);
  } else if (role == Qt::ForegroundRole) {
    return formatColor(number, flags);
  } else {
    return QString::fromStdString(number.str());
  }
}

} // namespace util
} // namespace models
} // namespace pvui
