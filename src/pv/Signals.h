#ifndef PV_SIGNALS_H
#define PV_SIGNALS_H

#include "boost/signals2.hpp"

namespace pv {

template <typename Slot>
using Signal =
    typename boost::signals2::signal_type<Slot,
                                          boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>>::type;
using Connection = boost::signals2::connection;
using ScopedConnection = boost::signals2::scoped_connection;

} // namespace pv

#endif // PV_SIGNALS_H
