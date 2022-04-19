#include "SecurityUtils.h"

constexpr int maximumSymbolLength = 10;
const QRegularExpression invalidSymbolRegularExpression =
    QRegularExpression("[^A-Z0-9.]");

namespace pvui {
namespace util {
QValidator::State SecuritySymbolValidator::validate(QString &input,
                                                    int &pos) const {
  input = input.trimmed().toUpper();
  if (input.length() > maximumSymbolLength)
    return QValidator::State::Invalid;
  return input.contains(invalidSymbolRegularExpression)
             ? QValidator::State::Invalid
             : QValidator::State::Acceptable;
}
} // namespace util
} // namespace pvui
