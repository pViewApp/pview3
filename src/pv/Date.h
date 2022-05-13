#ifndef PV_DATE_H
#define PV_DATE_H

#include <chrono>
#include <date/date.h>

namespace pv {

using Date = date::local_days;
using Year = date::year;
using Month = date::month;
using Day = date::day;
using Days = date::days;
using Months = date::months;
using Years = date::years;
using YearMonthDay = date::year_month_day;
using YearMonth = date::year_month;
using MonthDay = date::month_day;

namespace dates {

inline Date today() {
  return Date(YearMonthDay(std::chrono::time_point_cast<date::days>(std::chrono::system_clock::now())));
}

} // namespace dates

} // namespace pv

#endif // PV_DATE_H
