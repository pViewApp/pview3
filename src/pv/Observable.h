#ifndef PV_OBSERVABLE_H
#define PV_OBSERVABLE_H

#include <boost/signals2/signal.hpp>


namespace pv {

template <typename T>
class Observable
{
private:
  T value;
  boost::signals2::signal<const T&, const T&
public:
  Observable();
  operator=(T newValue) {
    value = newValue;
  }
};

} // namespace pv

#endif // PV_OBSERVABLE_H
