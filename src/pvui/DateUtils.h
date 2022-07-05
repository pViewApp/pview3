#include "pv/Integer64.h"
#include <QDate>
#include <Qt>

#ifndef PVUI_DATEUTILS_H
#define PVUI_DATEUTILS_H

namespace pvui {

pv::i64 toEpochDate(const QDate& date, Qt::TimeSpec TimeSpec = Qt::LocalTime, int offsetSeconds = 0);

pv::i64 currentEpochDate();

QDate toQDate(pv::i64 date, Qt::TimeSpec TimeSpec = Qt::LocalTime, int offsetSeconds = 0);

} // namespace pvui

#endif // PVUI_DATEUTILS_H
