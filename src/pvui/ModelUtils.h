#ifndef PVUI_FORMATTINGUTILS_H
#define PVUI_FORMATTINGUTILS_H

#include "pv/Decimal.h"
#include <QString>
#include <QVariant>

namespace pvui {
namespace models {
namespace util {

QVariant moneyData(pv::Decimal money, int role);
QVariant percentageData(pv::Decimal percentage, int role);

} // namespace util
} // namespace models
} // namespace pvui

#endif // PVUI_FORMATTINGUTILS_H
