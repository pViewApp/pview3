#ifndef PVUI_FORMATUTILS_H
#define PVUI_FORMATUTILS_H

#include "pv/Decimal.h"
#include <QLocale>
#include <QString>

namespace pvui {
namespace util {

QString formatMoney(const pv::Decimal& money);
QString formatPercentage(const pv::Decimal& percentage);

} // namespace util
} // namespace pvui

#endif // PVUI_FORMATUTILS_H
