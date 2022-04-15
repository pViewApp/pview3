#ifndef TYPES_H
#define TYPES_H

#include <date/date.h>
#include <boost/multiprecision/cpp_dec_float.hpp>

namespace pv {
	using Date = date::local_days;
	using Decimal = boost::multiprecision::cpp_dec_float_50;
}

#endif // TYPES_H
