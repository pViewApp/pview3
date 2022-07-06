#include "ModelUtils.h"
#include "FormatUtils.h"
#include "pv/Integer64.h"
#include <QColor>
#include <QStringLiteral>
#include <QVariant>
#include <Qt>
#include <limits>
#include <qvariant.h>
#include <string>
#include <type_traits>

namespace pvui {
namespace modelutils {

namespace {

QVariant formatColor(double amount, FormatFlags flags) {
  if (flags & FormatFlag::ColorNegative && amount < 0) {
    // Color negative number
    return QColor(Qt::GlobalColor::red);
  }
  if (flags & FormatFlag::ColorPositive && amount > 0) {
    return QColor(Qt::GlobalColor::darkGreen);
  } else {
    return QVariant(); // Don't color text
  }
}

QVariant formatAlignment(FormatFlags flags) {
  if (flags & FormatFlag::Numeric) {
    return QVariant(Qt::AlignRight | Qt::AlignVCenter);
  } else {
    return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
  }
}

QVariant sortData(const QVariant& value, FormatFlags flags) {
  if (flags & FormatFlag::SortFirst) {
    return -std::numeric_limits<double>::infinity();
  } else if (flags & FormatFlag::SortLast) {
    return std::numeric_limits<double>::infinity();
  } else {
    return value;
  }
}

template <typename NumberType> QVariant genericNumberData(NumberType number, int role, FormatFlags flags) {
  if (role == Qt::TextAlignmentRole) {
    return formatAlignment(flags);
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
  } else if (role == SortRole) {
    return sortData(number, flags);
  } else {
    return QVariant();
  }
}

} // namespace

QVariant stringData(const QString& string, int role, FormatFlags flags) {
  if (role == Qt::TextAlignmentRole) {
    return formatAlignment(flags);
  } else if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
    return string;
  } else if (role == SortRole) {
    return sortData(string, flags);
  } else {
    return QVariant();
  }
}

QVariant moneyData(pv::i64 money, int role, FormatFlags flags) {
  if (role == Qt::TextAlignmentRole) {
    return formatAlignment(flags);
  } else if (role == Qt::EditRole) {
    return money / 100.;
  } else if (role == Qt::ForegroundRole) {
    return formatColor(money, flags);
  } else if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
    return ::pvui::util::formatMoney(money);
  } else if (role == SortRole) {
    return sortData(money, flags);
  } else {
    return QVariant();
  }
}

QVariant percentageData(double percentage, int role, FormatFlags flags) {
  if (role == Qt::TextAlignmentRole) {
    return formatAlignment(flags);
  } else if (role == Qt::EditRole) {
    return percentage;
  } else if (role == Qt::ForegroundRole) {
    return formatColor(percentage, flags);
  } else if (role == SortRole) {
    return sortData(percentage, flags);
  } else if (role == Qt::DisplayRole || role == Qt::AccessibleTextRole) {
    return ::pvui::util::formatPercentage(percentage);
  } else {
    return QVariant();
  }
}

QVariant numberData(int number, int role, FormatFlags flags) { return genericNumberData(number, role, flags); }

QVariant numberData(pv::i64 number, int role, FormatFlags flags) { return genericNumberData(number, role, flags); }

QVariant numberData(double number, int role, FormatFlags flags) { return genericNumberData(number, role, flags); }

} // namespace modelutils
} // namespace pvui

