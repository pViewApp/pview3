#include "DateUtils.h"
#include "pv/Integer64.h"
#include <Qt>
#include <QDateTime>
#include <cmath>

namespace pvui {

pv::i64 toEpochDate(const QDate& date, Qt::TimeSpec timeSpec, int offsetSeconds) {
  return static_cast<pv::i64>(std::floor(QDateTime(date, QTime(23, 59, 59, 999), timeSpec, offsetSeconds).toSecsSinceEpoch() / 86400.0));
}

pv::i64 currentEpochDate() { return toEpochDate(QDate::currentDate()); }

QDate toQDate(pv::i64 date, Qt::TimeSpec timespec, int offsetSeconds) {
  QDateTime output;
  output.setOffsetFromUtc(offsetSeconds);
  output.setTimeSpec(timespec); // Make sure to set timespec after offset, to override any timespec changes from setOffsetFromUtc

  output.setSecsSinceEpoch(date * 86400);
  return output.date();
  
}

} // namespace pvui
