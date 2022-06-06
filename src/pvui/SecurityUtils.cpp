#include "SecurityUtils.h"
#include <QtGlobal>

#include <QRegularExpression>

namespace pvui {
namespace util {
QValidator::State SecuritySymbolValidator::validate(QString& input, int& /* pos */) const {
  constexpr int maximumSymbolLength = 10;

  static const QRegularExpression invalidSymbolRegularExpression = QRegularExpression("[^A-Z0-9.]");

  input = input.trimmed().toUpper();
  if (input.length() > maximumSymbolLength)
    return QValidator::State::Invalid;
  return input.contains(invalidSymbolRegularExpression) ? QValidator::State::Invalid : QValidator::State::Acceptable;
}
} // namespace util
} // namespace pvui
