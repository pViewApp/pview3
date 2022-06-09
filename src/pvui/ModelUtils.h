#ifndef PVUI_FORMATTINGUTILS_H
#define PVUI_FORMATTINGUTILS_H

#include "pv/Decimal.h"
#include <QFlags>
#include <QString>
#include <QVariant>

namespace pvui {
namespace models {
namespace util {

enum class FormatFlag {
  COLOR_NEGATIVE = 1,
  COLOR_POSITIVE = 2,
};

Q_DECLARE_FLAGS(FormatFlags, FormatFlag);
Q_DECLARE_OPERATORS_FOR_FLAGS(FormatFlags);

QVariant moneyData(pv::Decimal money, int role, FormatFlags flags = FormatFlags());
QVariant percentageData(pv::Decimal percentage, int role, FormatFlags flags = FormatFlags());
QVariant numberData(pv::Decimal number, int role, FormatFlags flags = FormatFlags());

} // namespace util
} // namespace models
} // namespace pvui

#endif // PVUI_FORMATTINGUTILS_H
