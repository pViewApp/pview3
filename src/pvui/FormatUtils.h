#ifndef PVUI_FORMATUTILS_H
#define PVUI_FORMATUTILS_H

#include "pv/Integer64.h"
#include <QLocale>
#include <QString>

namespace pvui {
namespace util {

QString formatMoney(pv::i64 money);
QString formatPercentage(double percentage);

} // namespace util
} // namespace pvui

#endif // PVUI_FORMATUTILS_H
