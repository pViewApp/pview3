#include "ModelUtils.h"
#include "FormatUtils.h"
#include "pv/Integer64.h"
#include <QColor>
#include <QStringLiteral>
#include <Qt>
#include <string>
#include <type_traits>

namespace pvui {
namespace modelutils {

namespace {

QVariant formatColor(double amount, const FormatFlags& flags) {
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

template <typename NumberType> QVariant genericNumberData(NumberType number, int role, FormatFlags flags) {
  if (role == Qt::TextAlignmentRole) {
    return QVariant(Qt::AlignTrailing | Qt::AlignCenter);
  } else if (role == Qt::EditRole) {
    // Cast to int or double, because only these types are editable with Qt's default item editor factory
    if (std::is_floating_point<NumberType>::value) {
      return static_cast<double>(number);
    } else {
      return static_cast<int>(number);
    }
  } else if (role == Qt::ForegroundRole) {
    return formatColor(number, flags);
  } else if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
    return QStringLiteral("%L1").arg(number); // return localized number
  } else {
    return QVariant();
  }
}

} // namespace

QVariant moneyData(pv::i64 money, int role, FormatFlags flags) {
  if (role == Qt::TextAlignmentRole) {
    return QVariant(Qt::AlignTrailing | Qt::AlignCenter);
  } else if (role == Qt::EditRole) {
    return money / 100.;
  } else if (role == Qt::ForegroundRole) {
    return formatColor(money, flags);
  } else if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
    return ::pvui::util::formatMoney(money);
  } else {
    return QVariant();
  }
}

QVariant percentageData(double percentage, int role, FormatFlags flags) {
  if (role == Qt::TextAlignmentRole) {
    return QVariant(Qt::AlignTrailing | Qt::AlignCenter);
  } else if (role == Qt::EditRole) {
    return percentage;
  } else if (role == Qt::ForegroundRole) {
    return formatColor(percentage, flags);
  } else {
    return ::pvui::util::formatPercentage(percentage);
  }
}

QVariant numberData(int number, int role, FormatFlags flags) { return genericNumberData(number, role, flags); }

QVariant numberData(pv::i64 number, int role, FormatFlags flags) { return genericNumberData(number, role, flags); }

QVariant numberData(double number, int role, FormatFlags flags) { return genericNumberData(number, role, flags); }

} // namespace modelutils
} // namespace pvui

