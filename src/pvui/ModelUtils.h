#ifndef PVUI_FORMATTINGUTILS_H
#define PVUI_FORMATTINGUTILS_H

#include "pv/Integer64.h"
#include <QFlags>
#include <QString>
#include <QVariant>

namespace pvui {
namespace modelutils {

enum class FormatFlag {
  COLOR_NEGATIVE = 1,
  COLOR_POSITIVE = 2,
};

Q_DECLARE_FLAGS(FormatFlags, FormatFlag);
Q_DECLARE_OPERATORS_FOR_FLAGS(FormatFlags);

QVariant moneyData(pv::i64 money, int role, FormatFlags flags = FormatFlags());
QVariant percentageData(double percentage, int role, FormatFlags flags = FormatFlags());

QVariant numberData(int number, int role, FormatFlags flags = FormatFlags());
QVariant numberData(pv::i64 number, int role, FormatFlags flags = FormatFlags());

QVariant numberData(double number, int role, FormatFlags flags = FormatFlags());

} // namespace modelutils
} // namespace pvui

#endif // PVUI_FORMATTINGUTILS_H
