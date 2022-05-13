#ifndef PV_INVALIDATABLE_H
#define PV_INVALIDATABLE_H

#include <boost/signals2/signal.hpp>

namespace pv {

class Invalidatable {
protected:
  explicit Invalidatable(){};

public:
  virtual ~Invalidatable(){};
  virtual boost::signals2::signal<void()>& invalidated() const noexcept = 0;
};

} // namespace pv

#endif // PV_INVALIDATABLE_H
