#ifndef PVUI_FORMATTINGUTILS_H
#define PVUI_FORMATTINGUTILS_H

#include "pv/Integer64.h"
#include <QFlags>
#include <QString>
#include <QVariant>

namespace pvui {
namespace modelutils {

enum class FormatFlag {
  ColorNegative = 1,
  ColorPositive = 2,
  /// Format as a numeric value.
  /// This is currently implemented by right-aligning the value.
  Numeric = 4,
};

Q_DECLARE_FLAGS(FormatFlags, FormatFlag);
Q_DECLARE_OPERATORS_FOR_FLAGS(FormatFlags);

/// \brief a Qt role used to get values for sorting
constexpr int SortRole = 2008;

QVariant stringData(const QString& string, int role, FormatFlags flags = FormatFlags());

QVariant moneyData(pv::i64 money, int role, FormatFlags flags = FormatFlag::Numeric);
QVariant percentageData(double percentage, int role, FormatFlags flags = FormatFlag::Numeric);

QVariant numberData(int number, int role, FormatFlags flags = FormatFlag::Numeric);
QVariant numberData(pv::i64 number, int role, FormatFlags flags = FormatFlag::Numeric);

QVariant numberData(double number, int role, FormatFlags flags = FormatFlag::Numeric);

} // namespace modelutils
} // namespace pvui

#endif // PVUI_FORMATTINGUTILS_H
