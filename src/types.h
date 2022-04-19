#ifndef PV_TYPES_H
#define PV_TYPES_H

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <date/date.h>

namespace pv {
using Date = date::local_days;
using YearMonthDay = date::year_month_day;
using Year = date::year;
using Month = date::month;
using Day = date::day;

using Decimal = boost::multiprecision::cpp_dec_float_50;
} // namespace pv

#endif // PV_TYPES_H
