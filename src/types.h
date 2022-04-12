#pragma once
#ifndef TYPES_H
#define TYPES_H

#include <date/date.h>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/signals2.hpp>

namespace pv {
	using Date = date::local_days;
	using Decimal = boost::multiprecision::cpp_dec_float_50;
	template <typename T>
	using Signal = boost::signals2::signal<T>;
}

#endif // TYPES_H
