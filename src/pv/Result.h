#ifndef PV_RESULT_H
#define PV_RESULT_H

#include <variant>

namespace pv {

template <typename SuccessVal, typename ErrVal> class Result {
private:
  std::variant<SuccessVal, ErrVal> value;

public:
  Result(SuccessVal value) : value(value) {}
  Result(ErrVal value) : value(value) {}
  Result(std::variant<SuccessVal, ErrVal> value) : value(value) {}

  bool isSuccess() { return std::holds_alternative<SuccessVal>(value); }

  bool isError() { return std::holds_alternative<ErrVal>(value); }

  operator bool() { return isSuccess(); }

  /// Undefined behavior if isSuccess() == false.
  SuccessVal forceGet() const { return std::get<SuccessVal>(value); }

  /// Undefined behavior if isSuccess() == true.
  ErrVal forceGetError() const { return std::get<ErrVal>(value); }

  SuccessVal getOrElse(SuccessVal orElse) {
    if (isSuccess()) {
      return forceGet();
    } else {
      return orElse;
    }
  }
};

} // namespace pv

#endif // PV_RESULT_H
